#include "CameraThirdPerson.hpp"

#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "hooks/HookManager.hpp"
#include "network/NetworkTransport.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace noble {

namespace {

// haloreach+0x48A910: per-player camera-mode check. Verified prologue (MCC build 2025.08.16):
//   48 89 5C 24 08   mov qword ptr [rsp+8], rbx
//   48 89 74 24 10   mov qword ptr [rsp+10h], rsi
// the reference host hooks the same function (5-byte patch window) and answers it from its own camera state.
constexpr uintptr_t kCameraModeCheckRva = 0x48A910;
// haloreach+0xC6948 (unit_should_use_third_person_camera): checks whether a unit should use a 3rd-person camera
// (returns 1 with *outReason in 1..3 when seated in a vehicle, or calls +0x48A910 when on foot).
constexpr uintptr_t kUnitThirdPersonRva = 0xC6948;
// haloreach+0xC6EE8 (c_first_person_camera::update): updates the 1st-person camera (both on foot and in vehicles)
// and writes the 1st-person horizontal field of view in radians to [observerCommand + 0x28].
constexpr uintptr_t kFirstPersonUpdateRva = 0xC6EE8;
// haloreach+0x53EF8 (player_mapping_get_unit_by_output_user): maps output user index (0 = local player) to unit datum index.
constexpr uintptr_t kGetLocalUnitRva = 0x53EF8;
// haloreach+0x47044C (object_get_markers_by_string_id): looks up model markers (0xC2 = character head/camera marker) on an object.
constexpr uintptr_t kGetObjectMarkersRva = 0x47044C;
// haloreach+0x472184 (object_get_head_position): fallback that retrieves the world-space position of the unit's head bone node.
constexpr uintptr_t kGetUnitHeadPosRva = 0x472184;
// haloreach+0x473DC0 (object_get_ultimate_parent): walks parent_object_index (+0x14) to return the root vehicle datum index.
constexpr uintptr_t kGetUltimateParentRva = 0x473DC0;
// haloreach+0x477D9C (object_recompute_node_matrices): recomputes an object's simulation node matrices ([obj + 0x17A]) from its current parent.
constexpr uintptr_t kUpdateObjectNodesRva = 0x477D9C;
// haloreach+0x1DCADC (c_orbiting_camera::update) and haloreach+0x1E2028 (c_following_camera::update):
// both take (void* camera, int32_t userIndex, float dt, void* observerCommand) and write the third-person
// field of view in radians (from haloreach+0xC8640) to [observerCommand + 0x28].
constexpr uintptr_t kCameraFollowRva = 0x1DCADC;
constexpr uintptr_t kCameraFollowingUpdateRva = 0x1E2028;
constexpr uint8_t kExpectedPrologue[9] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24};
constexpr uint8_t kUnitThirdPersonPrologue[10] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10};
constexpr uint8_t kFirstPersonUpdatePrologue[11] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x48, 0x89, 0x68, 0x18};

using CameraModeCheckFn = bool(__fastcall*)(int32_t playerIndex);
using UnitThirdPersonFn = int32_t(__fastcall*)(int32_t unitDatumIndex, int32_t* outReason);
using GetLocalUnitFn = int32_t(__fastcall*)(int32_t userIndex);
using GetObjectMarkersFn = int16_t(__fastcall*)(int32_t objectDatumIndex, int32_t markerStringId, void* outMarkers,
                                                int16_t maxMarkers, uint8_t doNotWalkToParent, uint8_t useRenderNodes);
using GetUnitHeadPosFn = float*(__fastcall*)(int32_t unitDatumIndex, float* outPos);
using GetUltimateParentFn = int32_t(__fastcall*)(int32_t objectDatumIndex);
using UpdateObjectNodesFn = void(__fastcall*)(int32_t objectDatumIndex);
CameraModeCheckFn s_original = nullptr;
UnitThirdPersonFn s_origUnitThirdPerson = nullptr;
GetLocalUnitFn s_getLocalUnit = nullptr;
GetObjectMarkersFn s_getObjectMarkers = nullptr;
GetUnitHeadPosFn s_getUnitHeadPos = nullptr;
GetUltimateParentFn s_getUltimateParent = nullptr;
UpdateObjectNodesFn s_updateObjectNodes = nullptr;

std::atomic<bool> s_enabled{false};
std::atomic<bool> s_vehicleFirstPerson{false};
std::atomic<bool> s_vehicleFpKeyWasDown{false};
std::atomic<bool> s_localInVehicle{false};
std::atomic<int> s_distancePercent{100};   // 100 = engine default; >100 pulls the camera further back
std::atomic<int> s_followLogCount{0};
std::atomic<int> s_mode{0};   // experiment: 0 = answer true, 1 = answer false, 2 = original(...) | enabled
std::atomic<bool> s_f5WasDown{false};
std::atomic<int> s_logCount{0};
std::atomic<bool> s_engineThirdPerson{false};
std::atomic<bool> s_zoomHeld{false};
std::atomic<float> s_zoomBlend{0.0f};

using CameraUpdateFn = void(__fastcall*)(void* camera, int32_t userIndex, float dt, void* observerCommand);
CameraUpdateFn s_originalFollow = nullptr;
CameraUpdateFn s_originalFollowingUpdate = nullptr;
CameraUpdateFn s_origFirstPersonUpdate = nullptr;

int32_t __fastcall Hook_UnitThirdPerson(int32_t unitDatumIndex, int32_t* outReason) {
    int32_t dummyReason = 0;
    int32_t* reasonPtr = outReason ? outReason : &dummyReason;
    const int32_t orig = s_origUnitThirdPerson ? s_origUnitThirdPerson(unitDatumIndex, reasonPtr) : 0;
    if (s_getLocalUnit && unitDatumIndex != -1 && unitDatumIndex == s_getLocalUnit(0)) {
        const int32_t reason = *reasonPtr;
        const bool inVehicle = (reason >= 1 && reason <= 3);
        s_localInVehicle.store(inVehicle);
        if (inVehicle && s_vehicleFirstPerson.load()) {
            *reasonPtr = 0;
            return 0;
        }
    }
    return orig;
}

void __fastcall Hook_FirstPersonUpdate(void* camera, int32_t userIndex, float dt, void* observerCommand) {
    if (s_origFirstPersonUpdate) s_origFirstPersonUpdate(camera, userIndex, dt, observerCommand);
    if (userIndex != 0 || !observerCommand) return;
    const SettingsData settings = Settings::Get().Snapshot();
    const bool inVehicle = s_localInVehicle.load();

    // When in a vehicle in 1st person, unit_get_camera_position (haloreach+0x48A1C0) places the camera at the
    // vehicle's 3rd-person seat pivot marker instead of the character's head marker (string_id 0xC2).
    // Furthermore, in object_update (haloreach+0x475E3C), child objects (the seated pilot biped at 0x180476124)
    // update and store their c_interpolator node matrices (0x18047623E) BEFORE the parent vehicle updates its own
    // node matrices (0x18047615E) for the current tick T. Consequently, querying useRenderNodes=1 on the child unit
    // reads tick (T-1) matrices that lag 1 full tick behind the rendered vehicle (phasing backward into the Falcon's
    // rear passenger seats when flying).
    // Fix: refresh the pilot unit's simulation node matrices against the parent vehicle's current tick-T matrices,
    // compute the head's vehicle-local offset in tick-T space, and transform that offset by the vehicle's interpolated
    // render-frame root matrix (useRenderNodes=1) so the camera remains rigidly locked inside the cockpit at any speed.
    if (inVehicle) {
        const int32_t unitDatum = s_getLocalUnit ? s_getLocalUnit(0) : -1;
        if (unitDatum != -1) {
            auto* pos = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x04);
            const auto* fwd = reinterpret_cast<const float*>(static_cast<const uint8_t*>(observerCommand) + 0x2C);
            const auto* up  = reinterpret_cast<const float*>(static_cast<const uint8_t*>(observerCommand) + 0x38);

            const int32_t vehicleDatum = s_getUltimateParent ? s_getUltimateParent(unitDatum) : -1;
            if (vehicleDatum != -1 && vehicleDatum != unitDatum && s_updateObjectNodes) {
                s_updateObjectNodes(unitDatum);
            }

            alignas(16) uint8_t unitMarkerTick[0x80] = {};
            float headPos[3] = {pos[0], pos[1], pos[2]};
            bool gotHead = false;
            if (s_getObjectMarkers) {
                const int16_t count = s_getObjectMarkers(unitDatum, 0xC2, unitMarkerTick, 1, 1, 0);
                if (count > 0) {
                    const auto* mp = reinterpret_cast<const float*>(unitMarkerTick + 0x60);
                    headPos[0] = mp[0];
                    headPos[1] = mp[1];
                    headPos[2] = mp[2];
                    gotHead = true;
                }
            }
            if (!gotHead && s_getUnitHeadPos) {
                s_getUnitHeadPos(unitDatum, headPos);
                gotHead = true;
            }

            if (gotHead && vehicleDatum != -1 && vehicleDatum != unitDatum && s_getObjectMarkers) {
                alignas(16) uint8_t vehMarkerTick[0x80] = {};
                alignas(16) uint8_t vehMarkerRender[0x80] = {};
                const int16_t tickCount   = s_getObjectMarkers(vehicleDatum, 0, vehMarkerTick, 1, 1, 0);
                const int16_t renderCount = s_getObjectMarkers(vehicleDatum, 0, vehMarkerRender, 1, 1, 1);
                if (tickCount > 0 && renderCount > 0) {
                    // object_marker layout (haloreach+0x470560):
                    //   +0x38: real_matrix4x3 world_matrix (+0x38 scale, +0x3C forward[3], +0x48 left[3], +0x54 up[3], +0x60 pos[3])
                    const auto* vehFwdTick  = reinterpret_cast<const float*>(vehMarkerTick + 0x3C);
                    const auto* vehLeftTick = reinterpret_cast<const float*>(vehMarkerTick + 0x48);
                    const auto* vehUpTick   = reinterpret_cast<const float*>(vehMarkerTick + 0x54);
                    const auto* vehPosTick  = reinterpret_cast<const float*>(vehMarkerTick + 0x60);

                    const auto* vehFwdRender  = reinterpret_cast<const float*>(vehMarkerRender + 0x3C);
                    const auto* vehLeftRender = reinterpret_cast<const float*>(vehMarkerRender + 0x48);
                    const auto* vehUpRender   = reinterpret_cast<const float*>(vehMarkerRender + 0x54);
                    const auto* vehPosRender  = reinterpret_cast<const float*>(vehMarkerRender + 0x60);

                    const float dx = headPos[0] - vehPosTick[0];
                    const float dy = headPos[1] - vehPosTick[1];
                    const float dz = headPos[2] - vehPosTick[2];

                    const float localFwd  = dx * vehFwdTick[0]  + dy * vehFwdTick[1]  + dz * vehFwdTick[2];
                    const float localLeft = dx * vehLeftTick[0] + dy * vehLeftTick[1] + dz * vehLeftTick[2];
                    const float localUp   = dx * vehUpTick[0]   + dy * vehUpTick[1]   + dz * vehUpTick[2];

                    headPos[0] = vehPosRender[0] + vehFwdRender[0] * localFwd + vehLeftRender[0] * localLeft + vehUpRender[0] * localUp;
                    headPos[1] = vehPosRender[1] + vehFwdRender[1] * localFwd + vehLeftRender[1] * localLeft + vehUpRender[1] * localUp;
                    headPos[2] = vehPosRender[2] + vehFwdRender[2] * localFwd + vehLeftRender[2] * localLeft + vehUpRender[2] * localUp;
                }
            }

            const float offX = std::clamp(settings.vehicleFirstPersonOffsetX, -1.0f, 1.0f);
            const float offY = std::clamp(settings.vehicleFirstPersonOffsetY, -1.0f, 1.0f);
            // Camera right basis vector in Reach's right-handed world space: right = normalize(cross(forward, up)).
            float rightX = fwd[1] * up[2] - fwd[2] * up[1];
            float rightY = fwd[2] * up[0] - fwd[0] * up[2];
            float rightZ = fwd[0] * up[1] - fwd[1] * up[0];
            const float rightLen = std::sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
            if (rightLen > 1e-5f) {
                rightX /= rightLen;
                rightY /= rightLen;
                rightZ /= rightLen;
            }

            pos[0] = headPos[0] + rightX * (offX * 0.50f) + up[0] * (offY * 0.50f);
            pos[1] = headPos[1] + rightY * (offX * 0.50f) + up[1] * (offY * 0.50f);
            pos[2] = headPos[2] + rightZ * (offX * 0.50f) + up[2] * (offY * 0.50f);

            // c_first_person_camera::update (haloreach+0xC71B1..0xC71CC) populates:
            //   +0x88 (float[3] safe_position) with object_get_center_of_mass(vehicleDatum) (e.g. the Falcon's rear passenger cabin),
            //   +0xA4..0xB4 (unit pill base[3], height, radius) with the pilot unit's un-offset pill, and
            //   +0x00 (flags) |= 0x1801 (enabling bit 0x800 safe_position collision raycast).
            // During flight in observer_update_collision (haloreach+0xC8290), if observer_point_in_unit_pill (haloreach+0xC81C0)
            // finds the cockpit camera outside the stale pill at +0xA4, observer_collision_raycast (haloreach+0xCA2C4) raycasts
            // from +0x88 (the Falcon's rear cabin) to +0x04 (the cockpit), hitting the Falcon's interior bulkhead and snapping
            // the camera into the back seats! Fix by centering both safe_position (+0x88) and the unit pill (+0xA4..0xB4) on the
            // cockpit camera position and clearing flag 0x800.
            auto* cmdBytes   = static_cast<uint8_t*>(observerCommand);
            auto* cmdFlags   = reinterpret_cast<uint32_t*>(cmdBytes + 0x00);
            auto* safePos    = reinterpret_cast<float*>(cmdBytes + 0x88);
            auto* pillBase   = reinterpret_cast<float*>(cmdBytes + 0xA4);
            auto* pillHeight = reinterpret_cast<float*>(cmdBytes + 0xB0);
            auto* pillRadius = reinterpret_cast<float*>(cmdBytes + 0xB4);

            safePos[0] = pos[0];
            safePos[1] = pos[1];
            safePos[2] = pos[2];

            if (*pillHeight < 0.5f) *pillHeight = 1.0f;
            if (*pillRadius < 0.5f) *pillRadius = 0.5f;
            pillBase[0] = pos[0];
            pillBase[1] = pos[1];
            pillBase[2] = pos[2] - 0.5f * (*pillHeight);

            *cmdFlags = (*cmdFlags & ~0x800u) | 0x4000u;
        }
    }

    const float mod = std::clamp(inVehicle ? settings.vehicleFirstPersonFovMod : settings.firstPersonFovMod, -1.20f, 1.20f);
    if (std::fabs(mod) > 1e-4f) {
        auto* fovRad = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x28);
        // Positive mod (+0% .. +120%) widens FOV out up to 2.20x; negative mod (-0% .. -120%) narrows FOV in down to 1 / 2.20x.
        const float scale = (mod >= 0.0f) ? (1.0f + mod) : (1.0f / (1.0f - mod));
        // Clamp within 10 deg .. 145 deg (0.1745329f .. 2.5307274f rad) so Reach's FOV validator (1..150 deg) accepts it.
        *fovRad = std::clamp(*fovRad * scale, 0.1745329f, 2.5307274f);
    }
}

void ApplyThirdPersonCameraTuning(float dt, void* observerCommand) {
    if (!observerCommand) return;
    const bool active = s_enabled.load() || s_engineThirdPerson.load();
    const bool wantZoom = active && s_zoomHeld.load();
    const float step = std::clamp(dt > 0.0f && dt < 0.25f ? dt : (1.0f / 60.0f), 0.001f, 0.1f) * 10.0f;
    float blend = s_zoomBlend.load();
    if (wantZoom) {
        blend = std::min(1.0f, blend + step);
    } else {
        blend = std::max(0.0f, blend - step);
    }
    s_zoomBlend.store(blend);

    if (!active) return;
    const SettingsData settings = Settings::Get().Snapshot();

    // s_observer_command layout (verified at haloreach+0x1DE234 and haloreach+0xC9B0C):
    //   +0x10: float focus_offset[3] (x = forward, y = left [cross(up, forward)], z = up)
    //   +0x24: float focus_distance (distance behind the focus point)
    //   +0x28: float field_of_view (horizontal FOV in radians)
    auto* offsetLeftRight = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x14);
    auto* offsetUpDown    = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x18);
    auto* focusDistance   = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x24);
    auto* fovRad          = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x28);

    const float offX = std::clamp(settings.thirdPersonOffsetX, -1.0f, 1.0f);
    const float offY = std::clamp(settings.thirdPersonOffsetY, -1.0f, 1.0f);
    if (std::fabs(offX) > 1e-4f) {
        // +0x14 is along camera left (cross(up, forward)), so subtracting offX moves right when offX > 0 and left when offX < 0.
        *offsetLeftRight -= offX * 0.50f;
    }
    if (std::fabs(offY) > 1e-4f) {
        // +0x18 is along camera up, so adding offY moves up when offY > 0 and down when offY < 0.
        *offsetUpDown += offY * 0.50f;
    }

    const int percent = s_distancePercent.load();
    if (percent != 100) {
        *focusDistance *= static_cast<float>(percent) / 100.0f;
        if (s_followLogCount.fetch_add(1) < 3) {
            NOBLE_INFO("CameraThirdPerson: follow distance scaled {}% -> {:.3f}", percent, *focusDistance);
        }
    }
    if (blend > 0.001f) {
        const float scale = CameraThirdPerson::EffectiveFovScale();
        *fovRad = std::clamp(*fovRad * scale, 0.15f, 2.5f);
    }
}

void __fastcall Hook_CameraFollow(void* camera, int32_t userIndex, float dt, void* observerCommand) {
    if (s_originalFollow) s_originalFollow(camera, userIndex, dt, observerCommand);
    ApplyThirdPersonCameraTuning(dt, observerCommand);
}

void __fastcall Hook_CameraFollowingUpdate(void* camera, int32_t userIndex, float dt, void* observerCommand) {
    if (s_originalFollowingUpdate) s_originalFollowingUpdate(camera, userIndex, dt, observerCommand);
    ApplyThirdPersonCameraTuning(dt, observerCommand);
}

bool __fastcall Hook_CameraModeCheck(int32_t playerIndex) {
    const bool orig = s_original ? s_original(playerIndex) : false;
    if (playerIndex == 0) s_engineThirdPerson.store(orig);
    if (!s_enabled.load()) return orig;
    // Host/mode gate: online guests do not get the camera unless the session host runs it too (Tyler's rule). Offline and
    // the host always may.
    const bool online = noble::NetworkTransport::Get().IsActive();
    const bool allowed = !online || noble::NetworkTransport::Get().IsHost();
    if (!allowed) {
        static std::atomic<int> s_deniedLog{0};
        if (s_deniedLog.fetch_add(1) < 1) NOBLE_INFO("CameraThirdPerson: third person is host-only; guest keeps first person");
        return orig;
    }
    const int mode = s_mode.load();
    if (s_logCount.fetch_add(1) < 5) {
        NOBLE_INFO("CameraThirdPerson: camera check fired (player {}, mode {}, original {})", playerIndex, mode, orig);
    }
    if (mode == 1) return false;              // experiment: maybe true means "first person"
    if (mode == 2) return orig || true;       // same as true, kept for completeness
    return true;
}

} // namespace

bool CameraThirdPerson::Install(HMODULE haloreach) {
    const uintptr_t base = reinterpret_cast<uintptr_t>(haloreach);
    if (!base) return false;
    s_getLocalUnit = reinterpret_cast<GetLocalUnitFn>(base + kGetLocalUnitRva);
    s_getObjectMarkers = reinterpret_cast<GetObjectMarkersFn>(base + kGetObjectMarkersRva);
    s_getUnitHeadPos = reinterpret_cast<GetUnitHeadPosFn>(base + kGetUnitHeadPosRva);
    s_getUltimateParent = reinterpret_cast<GetUltimateParentFn>(base + kGetUltimateParentRva);
    s_updateObjectNodes = reinterpret_cast<UpdateObjectNodesFn>(base + kUpdateObjectNodesRva);

    void* target = reinterpret_cast<void*>(base + kCameraModeCheckRva);
    if (std::memcmp(target, kExpectedPrologue, sizeof(kExpectedPrologue)) != 0) {
        NOBLE_WARN("CameraThirdPerson: camera check at haloreach+0x{:X} does not match the expected prologue; third "
                   "person disabled", kCameraModeCheckRva);
        return false;
    }
    if (!HookManager::InstallHook(target, reinterpret_cast<void*>(&Hook_CameraModeCheck),
                                  reinterpret_cast<void**>(&s_original),
                                  "camera_mode_check")) {
        NOBLE_WARN("CameraThirdPerson: could not install the camera-mode detour");
        return false;
    }
    NOBLE_INFO("CameraThirdPerson: third-person camera check hooked at haloreach+0x{:X}", kCameraModeCheckRva);

    void* unitTpTarget = reinterpret_cast<void*>(base + kUnitThirdPersonRva);
    if (std::memcmp(unitTpTarget, kUnitThirdPersonPrologue, sizeof(kUnitThirdPersonPrologue)) == 0) {
        if (HookManager::InstallHook(unitTpTarget, reinterpret_cast<void*>(&Hook_UnitThirdPerson),
                                     reinterpret_cast<void**>(&s_origUnitThirdPerson), "unit_third_person")) {
            NOBLE_INFO("CameraThirdPerson: unit third-person hook installed at haloreach+0x{:X}", kUnitThirdPersonRva);
        } else {
            NOBLE_WARN("CameraThirdPerson: could not install unit third-person hook at haloreach+0x{:X}", kUnitThirdPersonRva);
        }
    } else {
        NOBLE_WARN("CameraThirdPerson: unit third-person function at haloreach+0x{:X} does not match expected prologue", kUnitThirdPersonRva);
    }

    void* fpUpdateTarget = reinterpret_cast<void*>(base + kFirstPersonUpdateRva);
    if (std::memcmp(fpUpdateTarget, kFirstPersonUpdatePrologue, sizeof(kFirstPersonUpdatePrologue)) == 0) {
        if (HookManager::InstallHook(fpUpdateTarget, reinterpret_cast<void*>(&Hook_FirstPersonUpdate),
                                     reinterpret_cast<void**>(&s_origFirstPersonUpdate), "first_person_camera_update")) {
            NOBLE_INFO("CameraThirdPerson: first-person camera update hook installed at haloreach+0x{:X}", kFirstPersonUpdateRva);
        } else {
            NOBLE_WARN("CameraThirdPerson: could not install first-person camera update hook at haloreach+0x{:X}", kFirstPersonUpdateRva);
        }
    } else {
        NOBLE_WARN("CameraThirdPerson: first-person camera update at haloreach+0x{:X} does not match expected prologue", kFirstPersonUpdateRva);
    }

    void* followTarget = reinterpret_cast<void*>(base + kCameraFollowRva);
    constexpr uint8_t kFollowPrologue[8] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x18, 0x55};
    if (std::memcmp(followTarget, kFollowPrologue, sizeof(kFollowPrologue)) == 0) {
        if (HookManager::InstallHook(followTarget, reinterpret_cast<void*>(&Hook_CameraFollow),
                                     reinterpret_cast<void**>(&s_originalFollow), "camera_follow")) {
            NOBLE_INFO("CameraThirdPerson: follow hook installed at haloreach+0x{:X}", kCameraFollowRva);
        } else {
            NOBLE_WARN("CameraThirdPerson: could not install the follow hook; distance tuning disabled");
        }
    } else {
        NOBLE_WARN("CameraThirdPerson: follow function at haloreach+0x{:X} does not match the expected prologue; "
                   "distance tuning disabled", kCameraFollowRva);
    }

    void* followingUpdateTarget = reinterpret_cast<void*>(base + kCameraFollowingUpdateRva);
    if (std::memcmp(followingUpdateTarget, kFollowPrologue, sizeof(kFollowPrologue)) == 0) {
        if (HookManager::InstallHook(followingUpdateTarget, reinterpret_cast<void*>(&Hook_CameraFollowingUpdate),
                                     reinterpret_cast<void**>(&s_originalFollowingUpdate), "camera_following_update")) {
            NOBLE_INFO("CameraThirdPerson: following-camera update hook installed at haloreach+0x{:X}", kCameraFollowingUpdateRva);
        } else {
            NOBLE_WARN("CameraThirdPerson: could not install following-camera update hook at haloreach+0x{:X}", kCameraFollowingUpdateRva);
        }
    } else {
        NOBLE_WARN("CameraThirdPerson: following-camera update at haloreach+0x{:X} does not match expected prologue", kCameraFollowingUpdateRva);
    }
    return true;
}

void CameraThirdPerson::SetDistancePercent(int percent) {
    s_distancePercent.store(std::clamp(percent, 40, 400));
    NOBLE_INFO("CameraThirdPerson: follow distance {}%", s_distancePercent.load());
}

void CameraThirdPerson::SetMode(int mode) {
    s_mode.store(mode);
    NOBLE_INFO("CameraThirdPerson: experiment mode {}", mode);
}

void CameraThirdPerson::SetEnabled(bool enabled) {
    const bool before = s_enabled.exchange(enabled);
    if (!enabled) {
        s_zoomHeld.store(false);
        s_zoomBlend.store(0.0f);
    }
    if (before != enabled) NOBLE_INFO("CameraThirdPerson: third person {}", enabled ? "on" : "off");
}

bool CameraThirdPerson::IsEnabled() { return s_enabled.load(); }
bool CameraThirdPerson::EngineWantsThirdPerson() { return s_engineThirdPerson.load(); }

void CameraThirdPerson::Toggle() { SetEnabled(!s_enabled.load()); }

void CameraThirdPerson::PollHotkey(bool f5Down) {
    const bool was = s_f5WasDown.exchange(f5Down);
    if (f5Down && !was) Toggle();
}

void CameraThirdPerson::SetVehicleFirstPerson(bool enabled) {
    const bool before = s_vehicleFirstPerson.exchange(enabled);
    if (before != enabled) {
        NOBLE_INFO("CameraThirdPerson: vehicle first person {}", enabled ? "on" : "off");
    }
}

bool CameraThirdPerson::IsVehicleFirstPerson() {
    return s_vehicleFirstPerson.load();
}

void CameraThirdPerson::ToggleVehicleFirstPerson() {
    SetVehicleFirstPerson(!s_vehicleFirstPerson.load());
}

void CameraThirdPerson::PollVehicleFirstPersonHotkey(bool keyDown) {
    const bool was = s_vehicleFpKeyWasDown.exchange(keyDown);
    if (keyDown && !was) {
        ToggleVehicleFirstPerson();
    }
}

bool CameraThirdPerson::IsInVehicle() {
    return s_localInVehicle.load();
}

void CameraThirdPerson::SetZoomHeld(bool held) {
    s_zoomHeld.store(held);
}

bool CameraThirdPerson::IsZoomHeld() {
    return s_zoomHeld.load();
}

float CameraThirdPerson::EffectiveFovScale() {
    if (!s_enabled.load() && !s_engineThirdPerson.load()) return 1.0f;
    const float blend = std::clamp(s_zoomBlend.load(), 0.0f, 1.0f);
    if (blend <= 0.001f) return 1.0f;
    const float zoomPercent = std::clamp(Settings::Get().Snapshot().thirdPersonZoom, 0.10f, 1.00f);
    // 50% zoom -> 0.65x FOV, 100% zoom -> 0.30x FOV, 10% zoom -> 0.93x FOV.
    const float targetScale = 1.0f - (zoomPercent * 0.70f);
    return std::clamp(1.0f + (targetScale - 1.0f) * blend, 0.25f, 1.0f);
}

} // namespace noble

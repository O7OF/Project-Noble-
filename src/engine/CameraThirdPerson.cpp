#include "CameraThirdPerson.hpp"

#include "DevCheats.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "hooks/HookManager.hpp"
#include "network/NetworkTransport.hpp"
#include "vfs/VFS.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <mutex>
#include <cstdio>
#include <cstring>
#include <intrin.h>

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
// haloreach+0x134B2C (player_control_get_aiming_position): computes the unit's actual simulation aiming origin and aiming vector.
constexpr uintptr_t kGetPlayerAimingPosRva = 0x134B2C;
// haloreach+0xCB3A0 (observer_get_collision_flags): returns 64-bit collision flags for observer/aim raycasts.
constexpr uintptr_t kGetObserverCollisionFlagsRva = 0xCB3A0;
// haloreach+0x12AFCC (collision_test_line): raycasts between two 3D world points against structure/terrain/objects.
constexpr uintptr_t kCollisionTestLineRva = 0x12AFCC;
// haloreach+0x2DB54C (chud_compute_anchor_point): computes 2D screen anchor matrices for CHUD widgets (anchorType 9, 17, 21 = crosshair/reticle).
constexpr uintptr_t kChudComputeAnchorPointRva = 0x2DB54C;
// haloreach+0x2E5954 (chud_project_view_point): projects a 3D camera-view-space vector to 2D CHUD clip coordinates.
constexpr uintptr_t kChudProjectViewVectorRva = 0x2E5954;
// haloreach+0x2D8A04 (chud_map_projected_to_normalized): maps 2D CHUD clip coordinates through lens distortion to normalized [0..1] screen coords.
constexpr uintptr_t kChudMapProjectedToNormalizedRva = 0x2D8A04;
// haloreach+0x1DCADC (c_orbiting_camera::update) and haloreach+0x1E2028 (c_following_camera::update):
// both take (void* camera, int32_t userIndex, float dt, void* observerCommand) and write the third-person
// field of view in radians (from haloreach+0xC8640) to [observerCommand + 0x28].
constexpr uintptr_t kCameraFollowRva = 0x1DCADC;
constexpr uintptr_t kCameraFollowingUpdateRva = 0x1E2028;
constexpr uint8_t kExpectedPrologue[9] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24};
constexpr uint8_t kUnitThirdPersonPrologue[10] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10};
constexpr uint8_t kFirstPersonUpdatePrologue[11] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x48, 0x89, 0x68, 0x18};
constexpr uint8_t kChudAnchorPrologue[11] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x08, 0x4C, 0x89, 0x48, 0x20};

using CameraModeCheckFn = int32_t(__fastcall*)(int32_t unitDatumIndex);
using UnitThirdPersonFn = int32_t(__fastcall*)(int32_t unitDatumIndex, int32_t* outReason);
using GetLocalUnitFn = int32_t(__fastcall*)(int32_t userIndex);
using GetObjectMarkersFn = int16_t(__fastcall*)(int32_t objectDatumIndex, int32_t markerStringId, void* outMarkers,
                                                int16_t maxMarkers, uint8_t doNotWalkToParent, uint8_t useRenderNodes);
using GetUnitHeadPosFn = float*(__fastcall*)(int32_t unitDatumIndex, float* outPos);
using GetUltimateParentFn = int32_t(__fastcall*)(int32_t objectDatumIndex);
using UpdateObjectNodesFn = void(__fastcall*)(int32_t objectDatumIndex);
using GetPlayerAimingPosFn = int32_t(__fastcall*)(int32_t unitDatumIndex, float* outAimPosition, float* outAimVector, int32_t useDesiredAiming);
using GetObserverCollisionFlagsFn = uint64_t(__fastcall*)(int32_t userIndex);
using CollisionTestLineFn = uint8_t(__fastcall*)(uint64_t collisionFlags, uint32_t secondaryFlags, const float* startPoint,
                                                 const float* endPoint, int32_t ignoreObject1, int32_t ignoreObject2,
                                                 int32_t ignoreObject3, void* outCollisionResult);
using ChudComputeAnchorPointFn = uint8_t(__fastcall*)(int32_t chudUserIndex, int32_t anchorType, void* widgetData,
                                                      void* arg4, float* outAnchorMatrix, uint8_t* outClampFlag);
using ChudProjectViewVectorFn = uint8_t(__fastcall*)(const float* viewSpaceVec, float* outProjXY);
using ChudMapProjectedToNormalizedFn = void(__fastcall*)(void* unusedRcx, const float* inProjXY, float* outNormX, float* outNormY);
uintptr_t s_reachBase = 0;
CameraModeCheckFn s_original = nullptr;
UnitThirdPersonFn s_origUnitThirdPerson = nullptr;
GetLocalUnitFn s_getLocalUnit = nullptr;
GetObjectMarkersFn s_getObjectMarkers = nullptr;
GetUnitHeadPosFn s_getUnitHeadPos = nullptr;
GetUltimateParentFn s_getUltimateParent = nullptr;
UpdateObjectNodesFn s_updateObjectNodes = nullptr;
GetPlayerAimingPosFn s_getPlayerAimingPos = nullptr;
GetObserverCollisionFlagsFn s_getObserverCollisionFlags = nullptr;
CollisionTestLineFn s_collisionTestLine = nullptr;
ChudComputeAnchorPointFn s_origChudComputeAnchorPoint = nullptr;
ChudProjectViewVectorFn s_chudProjectViewVector = nullptr;
ChudMapProjectedToNormalizedFn s_chudMapProjectedToNormalized = nullptr;

std::atomic<bool> s_enabled{false};
std::atomic<bool> s_locked{false};
std::atomic<bool> s_vehicleFirstPerson{false};
std::atomic<bool> s_vehicleFpKeyWasDown{false};
std::atomic<bool> s_localInVehicle{false};
std::atomic<int> s_distancePercent{100};   // 100 = engine default; >100 pulls the camera further back
std::atomic<int> s_followLogCount{0};
std::atomic<int> s_mode{0};   // experiment: 0 = answer true, 1 = answer false, 2 = original(...) | enabled
std::atomic<bool> s_f5WasDown{false};
std::atomic<int> s_logCount{0};
std::atomic<bool> s_engineThirdPerson{false};
std::atomic<uint64_t> s_frameCounter{0};

// Camera hooks run every frame. Settings::Get().Snapshot() copies the whole SettingsData (strings, maps) under a
// mutex, so reading it per frame allocated several times per camera update and caused hitching in a match. Cache
// only the camera fields here and refresh them 4x/sec.
struct CameraTuning {
    float thirdOffsetX = 0.0f, thirdOffsetY = 0.0f;
    float vehicleOffsetX = 0.0f, vehicleOffsetY = 0.0f;
    float firstPersonFovMod = 0.0f, vehicleFirstPersonFovMod = 0.0f;
    float thirdPersonZoom = 0.50f;
};
std::mutex s_tuningMutex;
CameraTuning s_tuning;
std::atomic<uint64_t> s_tuningPollMs{0};

CameraTuning CurrentTuning() {
    const uint64_t now = GetTickCount64();
    if (now - s_tuningPollMs.load() >= 250) {
        s_tuningPollMs.store(now);
        const SettingsData settings = Settings::Get().Snapshot();
        CameraTuning next;
        next.thirdOffsetX = settings.thirdPersonOffsetX;
        next.thirdOffsetY = settings.thirdPersonOffsetY;
        next.vehicleOffsetX = settings.vehicleFirstPersonOffsetX;
        next.vehicleOffsetY = settings.vehicleFirstPersonOffsetY;
        next.firstPersonFovMod = settings.firstPersonFovMod;
        next.vehicleFirstPersonFovMod = settings.vehicleFirstPersonFovMod;
        next.thirdPersonZoom = settings.thirdPersonZoom;
        std::lock_guard<std::mutex> lock(s_tuningMutex);
        s_tuning = next;
    }
    std::lock_guard<std::mutex> lock(s_tuningMutex);
    return s_tuning;
}
std::atomic<bool> s_zoomHeld{false};
std::atomic<float> s_zoomBlend{0.0f};
std::mutex s_zoomMutex;
LARGE_INTEGER s_zoomLastQpc{};
LARGE_INTEGER s_zoomQpcFreq{};
bool s_zoomQpcInit = false;

float AdvanceZoomBlend(bool wantZoom) {
    std::lock_guard<std::mutex> lock(s_zoomMutex);
    if (!s_zoomQpcInit) {
        QueryPerformanceFrequency(&s_zoomQpcFreq);
        QueryPerformanceCounter(&s_zoomLastQpc);
        s_zoomQpcInit = true;
    }
    LARGE_INTEGER now{};
    QueryPerformanceCounter(&now);
    float qpcDt = 0.0f;
    if (s_zoomQpcFreq.QuadPart > 0 && now.QuadPart >= s_zoomLastQpc.QuadPart) {
        qpcDt = static_cast<float>(
            static_cast<double>(now.QuadPart - s_zoomLastQpc.QuadPart) /
            static_cast<double>(s_zoomQpcFreq.QuadPart));
    }
    s_zoomLastQpc = now;
    // Clamp large pauses (alt-tab, pause screen, loading) so zoom resumes cleanly without snapping, and cap
    // individual steps at 50ms so frame-rate independent smoothstep remains silky on variable FPS systems.
    if (!std::isfinite(qpcDt) || qpcDt < 0.0f || qpcDt > 0.25f) {
        qpcDt = 0.0f;
    } else if (qpcDt > 0.05f) {
        qpcDt = 0.05f;
    }

    float blend = s_zoomBlend.load(std::memory_order_relaxed);
    if (!std::isfinite(blend)) blend = 0.0f;
    blend = std::clamp(blend, 0.0f, 1.0f);

    const float target = wantZoom ? 1.0f : 0.0f;
    if (std::fabs(target - blend) <= 0.0005f) {
        blend = target;
    } else if (qpcDt > 0.0f) {
        const float rate = wantZoom ? 6.5f : 7.5f;
        blend = wantZoom
            ? std::min(1.0f, blend + qpcDt * rate)
            : std::max(0.0f, blend - qpcDt * rate);
    }
    s_zoomBlend.store(blend, std::memory_order_relaxed);
    return blend;
}

void ResetZoomBlendImmediate() {
    std::lock_guard<std::mutex> lock(s_zoomMutex);
    s_zoomBlend.store(0.0f, std::memory_order_relaxed);
    s_zoomQpcInit = false;
}

using CameraUpdateFn = void(__fastcall*)(void* camera, int32_t userIndex, float dt, void* observerCommand);
CameraUpdateFn s_originalFollow = nullptr;
CameraUpdateFn s_originalFollowingUpdate = nullptr;
CameraUpdateFn s_origFirstPersonUpdate = nullptr;

// Return-address RVAs of the local camera-director and FP-arms-visibility callers of haloreach+0xC6948:
//   - 0x1348B6: c_game_director::update (local camera mode selection: 0 = following_camera, 3 = first_person_camera)
//   - 0x1DAFF0: c_saved_film_director::update (local camera mode selection)
//   - 0x1DB656: first_person_weapon_should_draw (local 1st-person arms/weapon model visibility)
// Any other caller (specifically 0x134B53 in player_control_get_aiming_position, which computes the unit's
// simulation aiming/projectile origin) MUST receive the unmodified engine result so local 3rd-person view
// never alters simulation state or causes co-op / multiplayer desync.
bool IsLocalCameraOrRenderCaller(uintptr_t callerRva) {
    return callerRva == 0x1348B6 || callerRva == 0x1DAFF0 || callerRva == 0x1DB656;
}

int32_t __fastcall Hook_UnitThirdPerson(int32_t unitDatumIndex, int32_t* outReason) {
    int32_t dummyReason = 0;
    int32_t* reasonPtr = outReason ? outReason : &dummyReason;
    const int32_t orig = s_origUnitThirdPerson ? s_origUnitThirdPerson(unitDatumIndex, reasonPtr) : 0;
    const int32_t localUnit = s_getLocalUnit ? s_getLocalUnit(0) : -1;
    if (localUnit == -1) {
        s_localInVehicle.store(false);
        return orig;
    }
    if (unitDatumIndex == localUnit) {
        const int32_t reason = *reasonPtr;
        const bool inVehicle = (reason >= 1 && reason <= 3);
        s_localInVehicle.store(inVehicle);
        const uintptr_t retAddr = reinterpret_cast<uintptr_t>(_ReturnAddress());
        const uintptr_t callerRva = (s_reachBase && retAddr > s_reachBase) ? (retAddr - s_reachBase) : 0;
        if (IsLocalCameraOrRenderCaller(callerRva)) {
            if (inVehicle && s_vehicleFirstPerson.load() && !s_locked.load()) {
                *reasonPtr = 0;
                return 0;
            }
            if (!inVehicle && s_enabled.load() && s_mode.load() != 1) {
                if (*reasonPtr == 0) *reasonPtr = 4;
                return 1;
            }
        }
    }
    return orig;
}

void __fastcall Hook_FirstPersonUpdate(void* camera, int32_t userIndex, float dt, void* observerCommand) {
    if (s_origFirstPersonUpdate) s_origFirstPersonUpdate(camera, userIndex, dt, observerCommand);
    if (userIndex != 0 || !observerCommand) return;
    if (DevCheats::IsFreecamActive()) {
        auto* pos = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x04);
        const auto* fwd = reinterpret_cast<const float*>(static_cast<const uint8_t*>(observerCommand) + 0x2C);
        DevCheats::ApplyFreecam(pos, fwd);
        return;
    }
    const CameraTuning tuning = CurrentTuning();
    const int32_t unitDatum = s_getLocalUnit ? s_getLocalUnit(0) : -1;
    const int32_t vehicleDatum = (unitDatum != -1 && s_getUltimateParent) ? s_getUltimateParent(unitDatum) : -1;
    const bool actuallySeatedInVehicle = (unitDatum != -1 && vehicleDatum != -1 && vehicleDatum != unitDatum);
    if (unitDatum == -1) {
        s_localInVehicle.store(false);
    }
    const bool inVehicle = s_localInVehicle.load() && actuallySeatedInVehicle;
    // Vehicle first person off: the engine's third-person vehicle camera owns this update. Running the
    // cockpit-marker/offset work anyway burned a marker lookup (and, solo, a node recompute) every frame.
    if (inVehicle && (!s_vehicleFirstPerson.load() || s_locked.load()))
        return;

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
        auto* pos = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x04);
        const auto* fwd = reinterpret_cast<const float*>(static_cast<const uint8_t*>(observerCommand) + 0x2C);
        const auto* up  = reinterpret_cast<const float*>(static_cast<const uint8_t*>(observerCommand) + 0x38);

        const bool netActive = NetworkTransport::Get().IsActive();
        if (!netActive && s_updateObjectNodes) {
            s_updateObjectNodes(unitDatum);
        }

        alignas(16) uint8_t unitMarkerTick[0x80] = {};
        float headPos[3] = {pos[0], pos[1], pos[2]};
        bool gotHead = false;
        if (s_getObjectMarkers) {
            const int16_t count = s_getObjectMarkers(unitDatum, 0xC2, unitMarkerTick, 1, 1, netActive ? 1 : 0);
            if (count > 0) {
                const auto* mp = reinterpret_cast<const float*>(unitMarkerTick + 0x60);
                if (std::isfinite(mp[0]) && std::isfinite(mp[1]) && std::isfinite(mp[2])) {
                    headPos[0] = mp[0];
                    headPos[1] = mp[1];
                    headPos[2] = mp[2];
                    gotHead = true;
                }
            }
        }
        if (!gotHead && s_getUnitHeadPos) {
            float candidate[3] = {};
            s_getUnitHeadPos(unitDatum, candidate);
            if (std::isfinite(candidate[0]) && std::isfinite(candidate[1]) && std::isfinite(candidate[2])) {
                headPos[0] = candidate[0];
                headPos[1] = candidate[1];
                headPos[2] = candidate[2];
                gotHead = true;
            }
        }

        if (!netActive && gotHead && s_getObjectMarkers) {
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

                const float nextHead[3] = {
                    vehPosRender[0] + vehFwdRender[0] * localFwd + vehLeftRender[0] * localLeft + vehUpRender[0] * localUp,
                    vehPosRender[1] + vehFwdRender[1] * localFwd + vehLeftRender[1] * localLeft + vehUpRender[1] * localUp,
                    vehPosRender[2] + vehFwdRender[2] * localFwd + vehLeftRender[2] * localLeft + vehUpRender[2] * localUp,
                };
                if (std::isfinite(nextHead[0]) && std::isfinite(nextHead[1]) && std::isfinite(nextHead[2])) {
                    headPos[0] = nextHead[0];
                    headPos[1] = nextHead[1];
                    headPos[2] = nextHead[2];
                }
            }
        }

        const float offX = std::clamp(tuning.vehicleOffsetX, -1.0f, 1.0f);
        const float offY = std::clamp(tuning.vehicleOffsetY, -1.0f, 1.0f);
        if (std::isfinite(headPos[0]) && std::isfinite(headPos[1]) && std::isfinite(headPos[2]) &&
            std::isfinite(fwd[0]) && std::isfinite(fwd[1]) && std::isfinite(fwd[2]) &&
            std::isfinite(up[0])  && std::isfinite(up[1])  && std::isfinite(up[2])) {
            // Camera right basis vector in Reach's right-handed world space: right = normalize(cross(forward, up)).
            float rightX = fwd[1] * up[2] - fwd[2] * up[1];
            float rightY = fwd[2] * up[0] - fwd[0] * up[2];
            float rightZ = fwd[0] * up[1] - fwd[1] * up[0];
            const float rightLen = std::sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
            if (rightLen > 1e-5f) {
                rightX /= rightLen;
                rightY /= rightLen;
                rightZ /= rightLen;
            } else {
                rightX = rightY = rightZ = 0.0f;
            }

            const float nextPos[3] = {
                headPos[0] + rightX * (offX * 0.50f) + up[0] * (offY * 0.50f),
                headPos[1] + rightY * (offX * 0.50f) + up[1] * (offY * 0.50f),
                headPos[2] + rightZ * (offX * 0.50f) + up[2] * (offY * 0.50f),
            };
            if (std::isfinite(nextPos[0]) && std::isfinite(nextPos[1]) && std::isfinite(nextPos[2])) {
                pos[0] = nextPos[0];
                pos[1] = nextPos[1];
                pos[2] = nextPos[2];

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

                if (!std::isfinite(*pillHeight) || *pillHeight < 0.5f) *pillHeight = 1.0f;
                if (!std::isfinite(*pillRadius) || *pillRadius < 0.5f) *pillRadius = 0.5f;
                pillBase[0] = pos[0];
                pillBase[1] = pos[1];
                pillBase[2] = pos[2] - 0.5f * (*pillHeight);

                *cmdFlags = (*cmdFlags & ~0x800u) | 0x4000u;
            }
        }
    }

    const float mod = std::clamp(inVehicle ? tuning.vehicleFirstPersonFovMod : tuning.firstPersonFovMod, -1.20f, 1.20f);
    if (std::fabs(mod) > 1e-4f) {
        auto* fovRad = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x28);
        if (std::isfinite(*fovRad) && *fovRad > 0.01f) {
            // Positive mod (+0% .. +120%) widens FOV out up to 2.20x; negative mod (-0% .. -120%) narrows FOV in down to 1 / 2.20x.
            const float scale = (mod >= 0.0f) ? (1.0f + mod) : (1.0f / (1.0f - mod));
            // Clamp within 10 deg .. 145 deg (0.1745329f .. 2.5307274f rad) so Reach's FOV validator (1..150 deg) accepts it.
            *fovRad = std::clamp(*fovRad * scale, 0.1745329f, 2.5307274f);
        }
    }
}

// Mods that are third-person by default (the ODST-based Civil Unrest, for one) drive their own camera and aim
// zoom. When Noble's third person is off and a mod is active, the engine's own third-person request belongs to
// the mod: leave its camera alone instead of applying Noble's zoom, offset and distance on top of it.
bool NobleShapesCamera() {
    if (s_enabled.load()) return true;
    return s_engineThirdPerson.load() && !VFS::HasActiveMod();
}

thread_local int s_cameraHookDepth = 0;

struct CameraHookDepthGuard {
    int& depth;
    explicit CameraHookDepthGuard(int& d) : depth(d) { ++depth; }
    ~CameraHookDepthGuard() { --depth; }
};

void ApplyThirdPersonCameraTuning(float /*dt*/, void* observerCommand) {
    if (!observerCommand || s_cameraHookDepth != 1) return;

    // Vehicle 1st person owns +0x04..+0x38 and +0x28 on this update; decay third-person zoom cleanly and return.
    if (s_vehicleFirstPerson.load() && s_localInVehicle.load() && !s_locked.load()) {
        AdvanceZoomBlend(false);
        return;
    }

    // s_observer_command layout (verified at haloreach+0x1DE234 and haloreach+0xC9B0C):
    //   +0x10: float focus_offset[3] (x = forward, y = left [cross(up, forward)], z = up)
    //   +0x24: float focus_distance (distance behind the focus point)
    //   +0x28: float field_of_view (horizontal FOV in radians)
    auto* offsetLeftRight = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x14);
    auto* offsetUpDown    = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x18);
    auto* focusDistance   = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x24);
    auto* fovRad          = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x28);

    // Guard only against two sibling hooks running back-to-back on the exact same un-reset observerCommand
    // before the engine re-zeros and re-evaluates it; multi-tick frames (<60 FPS or frame dips) populate fresh
    // engine values each tick and must all be tuned every tick instead of skipping on s_frameCounter.
    thread_local void* s_lastTunedCommand = nullptr;
    thread_local bool s_hasLastTuned = false;
    thread_local float s_lastTunedLR = 0.0f;
    thread_local float s_lastTunedUD = 0.0f;
    thread_local float s_lastTunedDist = 0.0f;
    thread_local float s_lastTunedFov = 0.0f;

    if (s_hasLastTuned && observerCommand == s_lastTunedCommand &&
        *offsetLeftRight == s_lastTunedLR && *offsetUpDown == s_lastTunedUD &&
        *focusDistance == s_lastTunedDist && *fovRad == s_lastTunedFov) {
        return;
    }

    const bool active = NobleShapesCamera();
    const bool wantZoom = active && s_zoomHeld.load();
    const float blend = AdvanceZoomBlend(wantZoom);

    if (!active) {
        s_hasLastTuned = false;
        return;
    }

    if (!std::isfinite(*offsetLeftRight) || !std::isfinite(*offsetUpDown) ||
        !std::isfinite(*focusDistance) || !std::isfinite(*fovRad)) {
        s_hasLastTuned = false;
        return;
    }

    const float origLR = *offsetLeftRight;
    const float origUD = *offsetUpDown;
    const float origDist = *focusDistance;
    const float origFov = *fovRad;

    const CameraTuning tuning = CurrentTuning();
    const float offX = std::clamp(tuning.thirdOffsetX, -1.0f, 1.0f);
    const float offY = std::clamp(tuning.thirdOffsetY, -1.0f, 1.0f);
    if (std::fabs(offX) > 1e-4f) {
        // +0x14 is along camera left (cross(up, forward)), so subtracting offX moves right when offX > 0 and left when offX < 0.
        *offsetLeftRight -= offX * 0.50f;
    }
    if (std::fabs(offY) > 1e-4f) {
        // +0x18 is along camera up, so adding offY moves up when offY > 0 and down when offY < 0.
        *offsetUpDown += offY * 0.50f;
    }

    const int percent = s_distancePercent.load();
    if (percent != 100 && *focusDistance > 0.0f) {
        *focusDistance *= static_cast<float>(percent) / 100.0f;
        if (s_followLogCount.fetch_add(1) < 3) {
            NOBLE_INFO("CameraThirdPerson: follow distance scaled {}% -> {:.3f}", percent, *focusDistance);
        }
    }
    if (blend > 0.001f && *fovRad > 0.01f) {
        const float scale = CameraThirdPerson::EffectiveFovScale();
        *fovRad = std::clamp(*fovRad * scale, 0.1745329f, 2.5307274f);
    }

    const bool modified = (*offsetLeftRight != origLR || *offsetUpDown != origUD ||
                           *focusDistance != origDist || *fovRad != origFov);
    if (modified) {
        s_lastTunedCommand = observerCommand;
        s_lastTunedLR = *offsetLeftRight;
        s_lastTunedUD = *offsetUpDown;
        s_lastTunedDist = *focusDistance;
        s_lastTunedFov = *fovRad;
        s_hasLastTuned = true;
    } else {
        s_hasLastTuned = false;
    }
}

// Test-only "-third-person-front": mirror the third-person camera through its focus point so the player model
// (visor/armour) can be captured from the front. Runs after the engine camera update, never touches focus or mouse.
static void ApplyFrontTest(void* observerCommand) {
    static const bool s_frontTest = wcsstr(GetCommandLineW(), L"-third-person-front") != nullptr;
    if (!s_frontTest || !observerCommand) return;
    auto* pos = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x04);
    auto* fwd = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x2C);
    const float* focusDistance = reinterpret_cast<const float*>(static_cast<const uint8_t*>(observerCommand) + 0x24);
    float d = focusDistance[0];
    if (!std::isfinite(d) || d < 0.1f || d > 10.0f) d = 2.5f;
    for (int i = 0; i < 3; ++i) {
        pos[i] += 2.0f * d * fwd[i];
        fwd[i] = -fwd[i];
    }
}

void __fastcall Hook_CameraFollow(void* camera, int32_t userIndex, float dt, void* observerCommand) {
    CameraHookDepthGuard guard(s_cameraHookDepth);
    if (s_originalFollow) s_originalFollow(camera, userIndex, dt, observerCommand);
    if (userIndex != 0) return; // never write the local player's tuning onto another observer
    if (observerCommand && DevCheats::IsFreecamActive()) {
        auto* pos = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x04);
        const auto* fwd = reinterpret_cast<const float*>(static_cast<const uint8_t*>(observerCommand) + 0x2C);
        DevCheats::ApplyFreecam(pos, fwd);
        return;
    }
    ApplyThirdPersonCameraTuning(dt, observerCommand);
    ApplyFrontTest(observerCommand);
}

void __fastcall Hook_CameraFollowingUpdate(void* camera, int32_t userIndex, float dt, void* observerCommand) {
    CameraHookDepthGuard guard(s_cameraHookDepth);
    if (s_originalFollowingUpdate) s_originalFollowingUpdate(camera, userIndex, dt, observerCommand);
    if (userIndex != 0) return;
    if (observerCommand && DevCheats::IsFreecamActive()) {
        auto* pos = reinterpret_cast<float*>(static_cast<uint8_t*>(observerCommand) + 0x04);
        const auto* fwd = reinterpret_cast<const float*>(static_cast<const uint8_t*>(observerCommand) + 0x2C);
        DevCheats::ApplyFreecam(pos, fwd);
        return;
    }
    ApplyThirdPersonCameraTuning(dt, observerCommand);
    ApplyFrontTest(observerCommand);
}

int32_t __fastcall Hook_CameraModeCheck(int32_t unitDatumIndex) {
    const int32_t orig = s_original ? s_original(unitDatumIndex) : 0;
    const int32_t localUnit = s_getLocalUnit ? s_getLocalUnit(0) : -1;
    if (localUnit != -1 && unitDatumIndex == localUnit) {
        s_engineThirdPerson.store(orig != 0);
    }
    // When Hook_UnitThirdPerson (haloreach+0xC6948) is active, it selectively overrides only local camera-director
    // and FP-arms-visibility call sites while preserving simulation aiming (haloreach+0x134B4E) and weapon camera
    // marker checks (haloreach+0x48A527). Never override haloreach+0x48A910 directly when unit_third_person is hooked
    // or when localUnit is unknown (-1), as doing so alters simulation aiming and remote co-op units.
    if (s_origUnitThirdPerson != nullptr) return orig;
    if (!s_enabled.load()) return orig;
    if (localUnit == -1 || unitDatumIndex != localUnit) return orig;
    const int mode = s_mode.load();
    if (s_logCount.fetch_add(1) < 5) {
        NOBLE_INFO("CameraThirdPerson: camera check fired (unit 0x{:X}, mode {}, original {})",
                   static_cast<uint32_t>(unitDatumIndex), mode, orig);
    }
    if (mode == 1) return 0;                  // experiment: maybe non-zero means "first person"
    if (mode == 2) return orig != 0 ? orig : 1;
    return 1;
}

std::mutex s_dynCrosshairMutex;
uint64_t s_dynCachedFrame = 0;
bool s_dynCachedValid = false;
float s_dynCachedScreenX = 0.0f;
float s_dynCachedScreenY = 0.0f;
float s_dynSmoothedDist = 25.0f;
bool s_dynHasSmoothedDist = false;
LARGE_INTEGER s_dynLastQpc{};
LARGE_INTEGER s_dynQpcFreq{};
bool s_dynQpcInit = false;
std::atomic<int> s_dynLogCount{0};

void ResetDynamicCrosshairState() {
    std::lock_guard<std::mutex> lock(s_dynCrosshairMutex);
    s_dynCachedValid = false;
    s_dynHasSmoothedDist = false;
    s_dynQpcInit = false;
}

bool EvaluateDynamicThirdPersonCrosshair(float& outScreenX, float& outScreenY) {
    if (!s_reachBase || !s_getLocalUnit || !s_getPlayerAimingPos ||
        !s_getObserverCollisionFlags || !s_collisionTestLine ||
        !s_chudProjectViewVector || !s_chudMapProjectedToNormalized) {
        return false;
    }

    const uint64_t curFrame = s_frameCounter.load(std::memory_order_relaxed);
    std::lock_guard<std::mutex> lock(s_dynCrosshairMutex);
    if (curFrame != 0 && curFrame == s_dynCachedFrame) {
        if (s_dynCachedValid) {
            outScreenX = s_dynCachedScreenX;
            outScreenY = s_dynCachedScreenY;
        }
        return s_dynCachedValid;
    }
    s_dynCachedFrame = curFrame;
    s_dynCachedValid = false;

    const int32_t unitDatum = s_getLocalUnit(0);
    if (unitDatum == -1) {
        s_dynHasSmoothedDist = false;
        return false;
    }

    float aimOrigin[3] = {};
    float aimDir[3] = {};
    s_getPlayerAimingPos(unitDatum, aimOrigin, aimDir, 0);
    if (!std::isfinite(aimOrigin[0]) || !std::isfinite(aimOrigin[1]) || !std::isfinite(aimOrigin[2]) ||
        !std::isfinite(aimDir[0]) || !std::isfinite(aimDir[1]) || !std::isfinite(aimDir[2])) {
        return false;
    }

    const float aimLen = std::sqrt(aimDir[0] * aimDir[0] + aimDir[1] * aimDir[1] + aimDir[2] * aimDir[2]);
    if (aimLen <= 1e-4f) {
        return false;
    }
    aimDir[0] /= aimLen;
    aimDir[1] /= aimLen;
    aimDir[2] /= aimLen;

    constexpr float kMinTraceDist = 1.8f;
    constexpr float kDefaultConvergenceDist = 25.0f;
    constexpr float kMaxTraceDist = 100.0f;

    const float endPoint[3] = {
        aimOrigin[0] + aimDir[0] * kMaxTraceDist,
        aimOrigin[1] + aimDir[1] * kMaxTraceDist,
        aimOrigin[2] + aimDir[2] * kMaxTraceDist,
    };

    const int32_t rootParent = s_getUltimateParent ? s_getUltimateParent(unitDatum) : -1;
    const int32_t ignoreVehicle = (rootParent != -1 && rootParent != unitDatum) ? rootParent : -1;

    // Include structure/terrain/instanced geometry + gameplay object types (biped, vehicle, crate, machine, scenery, giant).
    const uint64_t collisionFlags = (s_getObserverCollisionFlags(0) & ~2ull) | (0x2985ull << 32);
    alignas(16) uint8_t collisionResult[0x100] = {};
    const uint8_t hit = s_collisionTestLine(collisionFlags, 0, aimOrigin, endPoint,
                                            unitDatum, ignoreVehicle, -1, collisionResult);

    float rawDist = kDefaultConvergenceDist;
    if (hit != 0) {
        const auto* hitPos = reinterpret_cast<const float*>(collisionResult + 0x08);
        if (std::isfinite(hitPos[0]) && std::isfinite(hitPos[1]) && std::isfinite(hitPos[2])) {
            const float dx = hitPos[0] - aimOrigin[0];
            const float dy = hitPos[1] - aimOrigin[1];
            const float dz = hitPos[2] - aimOrigin[2];
            const float d = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (std::isfinite(d) && d > 0.01f) {
                rawDist = d;
            }
        }
    }
    const float clampedDist = std::clamp(rawDist, kMinTraceDist, kMaxTraceDist);

    if (!s_dynQpcInit) {
        QueryPerformanceFrequency(&s_dynQpcFreq);
        QueryPerformanceCounter(&s_dynLastQpc);
        s_dynQpcInit = true;
    }
    LARGE_INTEGER now{};
    QueryPerformanceCounter(&now);
    float dt = 0.0f;
    if (s_dynQpcFreq.QuadPart > 0 && now.QuadPart >= s_dynLastQpc.QuadPart) {
        dt = static_cast<float>(
            static_cast<double>(now.QuadPart - s_dynLastQpc.QuadPart) /
            static_cast<double>(s_dynQpcFreq.QuadPart));
    }
    s_dynLastQpc = now;
    if (!std::isfinite(dt) || dt < 0.0f || dt > 0.25f) {
        dt = 0.016f;
    }

    if (!s_dynHasSmoothedDist || !std::isfinite(s_dynSmoothedDist)) {
        s_dynSmoothedDist = clampedDist;
        s_dynHasSmoothedDist = true;
    } else {
        const float alpha = std::clamp(1.0f - std::exp(-dt * 18.0f), 0.05f, 1.0f);
        s_dynSmoothedDist += (clampedDist - s_dynSmoothedDist) * alpha;
        s_dynSmoothedDist = std::clamp(s_dynSmoothedDist, kMinTraceDist, kMaxTraceDist);
    }

    const float targetWorld[3] = {
        aimOrigin[0] + aimDir[0] * s_dynSmoothedDist,
        aimOrigin[1] + aimDir[1] * s_dynSmoothedDist,
        aimOrigin[2] + aimDir[2] * s_dynSmoothedDist,
    };

    // Read local player 0's active s_observer_result from TLS[g_observer_globals] + 0x154 (verified at haloreach+0x2DBC39..0x2DBC69).
    const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_reachBase + 0xC17B18);
    const auto* tlsArray = reinterpret_cast<const uintptr_t*>(__readgsqword(0x58));
    const uintptr_t tlsSlotBase = tlsArray ? tlsArray[tlsIndex] : 0;
    const uintptr_t observerGlobals = tlsSlotBase ? *reinterpret_cast<const uintptr_t*>(tlsSlotBase + 0x688) : 0;
    if (!observerGlobals) {
        return false;
    }

    const auto* camPos = reinterpret_cast<const float*>(observerGlobals + 0x154);
    const auto* camFwd = reinterpret_cast<const float*>(observerGlobals + 0x17C);
    const auto* camUp  = reinterpret_cast<const float*>(observerGlobals + 0x188);
    if (!std::isfinite(camPos[0]) || !std::isfinite(camPos[1]) || !std::isfinite(camPos[2]) ||
        !std::isfinite(camFwd[0]) || !std::isfinite(camFwd[1]) || !std::isfinite(camFwd[2]) ||
        !std::isfinite(camUp[0])  || !std::isfinite(camUp[1])  || !std::isfinite(camUp[2])) {
        return false;
    }

    // Right-handed camera basis: camRight = normalize(cross(camFwd, camUp)).
    float camRight[3] = {
        camFwd[1] * camUp[2] - camFwd[2] * camUp[1],
        camFwd[2] * camUp[0] - camFwd[0] * camUp[2],
        camFwd[0] * camUp[1] - camFwd[1] * camUp[0],
    };
    const float rightLen = std::sqrt(camRight[0] * camRight[0] + camRight[1] * camRight[1] + camRight[2] * camRight[2]);
    if (rightLen <= 1e-5f) {
        return false;
    }
    camRight[0] /= rightLen;
    camRight[1] /= rightLen;
    camRight[2] /= rightLen;

    const float delta[3] = {
        targetWorld[0] - camPos[0],
        targetWorld[1] - camPos[1],
        targetWorld[2] - camPos[2],
    };
    const float dFwd   = delta[0] * camFwd[0]   + delta[1] * camFwd[1]   + delta[2] * camFwd[2];
    const float dRight = delta[0] * camRight[0] + delta[1] * camRight[1] + delta[2] * camRight[2];
    const float dUp    = delta[0] * camUp[0]    + delta[1] * camUp[1]    + delta[2] * camUp[2];
    if (!std::isfinite(dFwd) || dFwd <= 0.1f) {
        return false;
    }

    // Construct camera-view-space point (X = right, Y = up, Z = -forward = -1.0f) matching haloreach+0x2DBC4B..0x2DBC8A.
    const float viewVec[3] = {
        std::clamp(dRight / dFwd, -1.5f, 1.5f),
        std::clamp(dUp / dFwd, -1.5f, 1.5f),
        -1.0f,
    };

    float projXY[2] = {};
    s_chudProjectViewVector(viewVec, projXY);
    if (!std::isfinite(projXY[0]) || !std::isfinite(projXY[1])) {
        return false;
    }

    float normX = 0.5f;
    float normY = 0.5f;
    s_chudMapProjectedToNormalized(nullptr, projXY, &normX, &normY);
    if (!std::isfinite(normX) || !std::isfinite(normY)) {
        return false;
    }

    const float screenW = *reinterpret_cast<const float*>(s_reachBase + 0xD1F6F0);
    const float screenH = *reinterpret_cast<const float*>(s_reachBase + 0xD1F6F4);
    if (!std::isfinite(screenW) || !std::isfinite(screenH) || screenW <= 1.0f || screenH <= 1.0f) {
        return false;
    }

    outScreenX = std::clamp(screenW * normX, screenW * 0.10f, screenW * 0.90f);
    outScreenY = std::clamp(screenH * (1.0f - normY), screenH * 0.10f, screenH * 0.90f);

    s_dynCachedScreenX = outScreenX;
    s_dynCachedScreenY = outScreenY;
    s_dynCachedValid = true;

    if (s_dynLogCount.fetch_add(1) < 5) {
        NOBLE_INFO("CameraThirdPerson: dynamic 3P crosshair hit={} dist={:.2f}m view=({:.4f}, {:.4f}) screen=({:.1f}, {:.1f}) / ({:.0f}x{:.0f})",
                   static_cast<int>(hit), s_dynSmoothedDist, viewVec[0], viewVec[1],
                   outScreenX, outScreenY, screenW, screenH);
    }
    return true;
}

uint8_t __fastcall Hook_ChudComputeAnchorPoint(int32_t chudUserIndex, int32_t anchorType, void* widgetData,
                                               void* arg4, float* outAnchorMatrix, uint8_t* outClampFlag) {
    const uint8_t orig = s_origChudComputeAnchorPoint
        ? s_origChudComputeAnchorPoint(chudUserIndex, anchorType, widgetData, arg4, outAnchorMatrix, outClampFlag)
        : 0;
    if (!orig || chudUserIndex != 0 || !outAnchorMatrix) {
        return orig;
    }
    // Crosshair / weapon reticle anchor types in chud_compute_anchor_point (haloreach+0x2DB54C):
    //   9  = _chud_anchor_type_crosshair
    //   17 = secondary crosshair anchor
    //   21 = weapon reticle anchor (when *outClampFlag != 0; if 0, 3D lock-on tracking owns the anchor)
    if (anchorType != 9 && anchorType != 17 && anchorType != 21) {
        return orig;
    }
    if (anchorType == 21 && outClampFlag && *outClampFlag == 0) {
        return orig;
    }

    // Engage ONLY when 3rd-person camera is active (never in 1st person, vehicle 1st person, or freecam).
    const bool inVehicleFp = s_vehicleFirstPerson.load() && s_localInVehicle.load() && !s_locked.load();
    if (!NobleShapesCamera() || inVehicleFp || DevCheats::IsFreecamActive()) {
        ResetDynamicCrosshairState();
        return orig;
    }

    float dynX = 0.0f;
    float dynY = 0.0f;
    if (EvaluateDynamicThirdPersonCrosshair(dynX, dynY)) {
        outAnchorMatrix[10] = dynX; // +0x28: screen X
        outAnchorMatrix[11] = dynY; // +0x2C: screen Y
    }
    return orig;
}

} // namespace

void CameraThirdPerson::TickFrame() { s_frameCounter.fetch_add(1); }

bool CameraThirdPerson::Install(HMODULE haloreach) {
    const uintptr_t base = reinterpret_cast<uintptr_t>(haloreach);
    if (!base) return false;
    s_reachBase = base;
    s_getLocalUnit = reinterpret_cast<GetLocalUnitFn>(base + kGetLocalUnitRva);
    s_getObjectMarkers = reinterpret_cast<GetObjectMarkersFn>(base + kGetObjectMarkersRva);
    s_getUnitHeadPos = reinterpret_cast<GetUnitHeadPosFn>(base + kGetUnitHeadPosRva);
    s_getUltimateParent = reinterpret_cast<GetUltimateParentFn>(base + kGetUltimateParentRva);
    s_updateObjectNodes = reinterpret_cast<UpdateObjectNodesFn>(base + kUpdateObjectNodesRva);
    s_getPlayerAimingPos = reinterpret_cast<GetPlayerAimingPosFn>(base + kGetPlayerAimingPosRva);
    s_getObserverCollisionFlags = reinterpret_cast<GetObserverCollisionFlagsFn>(base + kGetObserverCollisionFlagsRva);
    s_collisionTestLine = reinterpret_cast<CollisionTestLineFn>(base + kCollisionTestLineRva);
    s_chudProjectViewVector = reinterpret_cast<ChudProjectViewVectorFn>(base + kChudProjectViewVectorRva);
    s_chudMapProjectedToNormalized = reinterpret_cast<ChudMapProjectedToNormalizedFn>(base + kChudMapProjectedToNormalizedRva);

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

    void* chudAnchorTarget = reinterpret_cast<void*>(base + kChudComputeAnchorPointRva);
    if (std::memcmp(chudAnchorTarget, kChudAnchorPrologue, sizeof(kChudAnchorPrologue)) == 0) {
        if (HookManager::InstallHook(chudAnchorTarget, reinterpret_cast<void*>(&Hook_ChudComputeAnchorPoint),
                                     reinterpret_cast<void**>(&s_origChudComputeAnchorPoint), "chud_compute_anchor_point")) {
            NOBLE_INFO("CameraThirdPerson: CHUD anchor hook installed at haloreach+0x{:X}", kChudComputeAnchorPointRva);
        } else {
            NOBLE_WARN("CameraThirdPerson: could not install CHUD anchor hook at haloreach+0x{:X}", kChudComputeAnchorPointRva);
        }
    } else {
        NOBLE_WARN("CameraThirdPerson: CHUD anchor function at haloreach+0x{:X} does not match expected prologue", kChudComputeAnchorPointRva);
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

void CameraThirdPerson::LockThirdPerson() {
    s_locked.store(true);
    s_vehicleFirstPerson.store(false);
    SetEnabled(true);
    NOBLE_INFO("CameraThirdPerson: locked to third person (Trooper mode)");
}

bool CameraThirdPerson::IsLocked() { return s_locked.load(); }

void CameraThirdPerson::SetEnabled(bool enabled) {
    if (!enabled && s_locked.load()) return;   // Trooper mode: third person only
    const bool before = s_enabled.exchange(enabled);
    if (!enabled) {
        s_zoomHeld.store(false);
        ResetZoomBlendImmediate();
    }
    if (before != enabled) NOBLE_INFO("CameraThirdPerson: third person {}", enabled ? "on" : "off");
}

bool CameraThirdPerson::IsEnabled() { return s_enabled.load(); }
bool CameraThirdPerson::EngineWantsThirdPerson() { return s_engineThirdPerson.load(); }

void CameraThirdPerson::Toggle() { SetEnabled(!s_enabled.load()); }

void CameraThirdPerson::PollHotkey(bool keyDown) {
    const bool was = s_f5WasDown.exchange(keyDown);
    if (keyDown && !was) Toggle();
}

void CameraThirdPerson::SetVehicleFirstPerson(bool enabled) {
    if (enabled && s_locked.load()) return;   // Trooper mode: third person only
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
    if (!NobleShapesCamera()) return 1.0f;
    if (s_vehicleFirstPerson.load() && s_localInVehicle.load() && !s_locked.load()) return 1.0f;
    const float blend = std::clamp(s_zoomBlend.load(std::memory_order_relaxed), 0.0f, 1.0f);
    if (blend <= 0.001f) return 1.0f;
    const float smooth = blend * blend * (3.0f - 2.0f * blend);
    const float zoomPercent = std::clamp(CurrentTuning().thirdPersonZoom, 0.10f, 1.00f);
    // 50% zoom -> 0.65x FOV, 100% zoom -> 0.30x FOV, 10% zoom -> 0.93x FOV.
    const float targetScale = 1.0f - (zoomPercent * 0.70f);
    return std::clamp(1.0f + (targetScale - 1.0f) * smooth, 0.25f, 1.0f);
}

} // namespace noble

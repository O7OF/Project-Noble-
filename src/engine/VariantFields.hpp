#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace noble {

// The engine's decoded c_game_variant is 0xFC04 bytes. These are the offsets proven by
// patch -> decode -> native-diff (docs/plans/variant-field-map.md). Only the fields below are mapped;
// loadouts are file/script data and are not here.
struct VariantFields {
    // General
    uint8_t roundsToWin = 0;
    uint8_t timeLimit = 0;
    uint8_t roundLimit = 0;
    uint8_t suddenDeath = 0;
    uint8_t gracePeriod = 0;
    uint8_t respawnFlags = 0;
    uint8_t livesPerRound = 0;
    uint8_t teamLivesPerRound = 0;
    uint8_t respawnTime = 0;
    uint8_t suicidePenalty = 0;
    uint8_t betrayalPenalty = 0;
    uint8_t respawnGrowth = 0;
    uint8_t loadoutCameraTime = 0;
    uint8_t traitsDuration = 0;
    uint8_t socialFlags = 0;
    uint8_t mapFlags = 0;
    uint16_t scoreToWin = 0;
    // Player traits
    uint8_t damageResist = 0;
    uint8_t healthMult = 0;
    uint8_t healthRate = 0;
    uint8_t shieldMult = 0;
    uint8_t shieldRate = 0;
    uint8_t shieldDelay = 0;
    uint8_t headshotImmune = 0;
    uint8_t vampirism = 0;
    uint8_t assassinImmune = 0;
    uint8_t weaponDamage = 0;
    uint8_t meleeDamage = 0;
    uint8_t grenadeRegen = 0;
    uint8_t weaponPickup = 0;
    uint8_t grenadeCount = 0;
    uint8_t infiniteAmmo = 0;
    uint8_t abilityDrop = 0;
    uint8_t infiniteAbility = 0;
    uint8_t abilityUsage = 0;
    uint8_t primaryWeapon = 0;
    uint8_t secondaryWeapon = 0;
    uint8_t ability = 0;
    uint8_t speed = 0;
    uint8_t gravity = 0;
    uint8_t vehicleUsage = 0;
    uint8_t activeCamo = 0;
    uint8_t waypoint = 0;
    uint8_t visibleName = 0;
    uint8_t aura = 0;
    uint8_t forcedColor = 0;
    uint8_t radarState = 0;
    uint8_t radarRange = 0;
    uint8_t damageIndicator = 0;

    static constexpr size_t kDecodedSize = 0xFC04;
    static constexpr size_t kMinSize = 0xF842;

    bool Read(const uint8_t* data, size_t size) {
        if (!data || size < kMinSize) return false;
        roundsToWin = data[0x2BD];
        timeLimit = data[0x2BE];
        roundLimit = data[0x2BF];
        suddenDeath = data[0x2C0];
        gracePeriod = data[0x2C2];
        respawnFlags = data[0x2C4];
        livesPerRound = data[0x2C5];
        teamLivesPerRound = data[0x2C6];
        respawnTime = data[0x2C7];
        suicidePenalty = data[0x2C8];
        betrayalPenalty = data[0x2C9];
        respawnGrowth = data[0x2CA];
        loadoutCameraTime = data[0x2CB];
        traitsDuration = data[0x2CC];
        socialFlags = data[0x2FC];
        mapFlags = data[0x300];
        std::memcpy(&scoreToWin, data + 0xF840, sizeof(scoreToWin));
        damageResist = data[0x2D0];
        healthMult = data[0x2D1];
        healthRate = data[0x2D2];
        shieldMult = data[0x2D3];
        shieldRate = data[0x2D4];
        shieldDelay = data[0x2D5];
        headshotImmune = data[0x2D6];
        vampirism = data[0x2D7];
        assassinImmune = data[0x2D8];
        weaponDamage = data[0x2DC];
        meleeDamage = data[0x2DD];
        grenadeRegen = data[0x2DE];
        weaponPickup = data[0x2DF];
        grenadeCount = data[0x2E0];
        infiniteAmmo = data[0x2E1];
        abilityDrop = data[0x2E2];
        infiniteAbility = data[0x2E3];
        abilityUsage = data[0x2E4];
        primaryWeapon = data[0x2E5];
        secondaryWeapon = data[0x2E6];
        ability = data[0x2E7];
        speed = data[0x2E8];
        gravity = data[0x2E9];
        vehicleUsage = data[0x2EA];
        activeCamo = data[0x2F0];
        waypoint = data[0x2F1];
        visibleName = data[0x2F2];
        aura = data[0x2F3];
        forcedColor = data[0x2F4];
        radarState = data[0x2F8];
        radarRange = data[0x2F9];
        damageIndicator = data[0x2FA];
        return true;
    }

    bool Write(uint8_t* data, size_t size) const {
        if (!data || size < kMinSize) return false;
        data[0x2BD] = roundsToWin;
        data[0x2BE] = timeLimit;
        data[0x2BF] = roundLimit;
        data[0x2C0] = suddenDeath;
        data[0x2C2] = gracePeriod;
        data[0x2C4] = respawnFlags;
        data[0x2C5] = livesPerRound;
        data[0x2C6] = teamLivesPerRound;
        data[0x2C7] = respawnTime;
        data[0x2C8] = suicidePenalty;
        data[0x2C9] = betrayalPenalty;
        data[0x2CA] = respawnGrowth;
        data[0x2CB] = loadoutCameraTime;
        data[0x2CC] = traitsDuration;
        data[0x2FC] = socialFlags;
        data[0x300] = mapFlags;
        std::memcpy(data + 0xF840, &scoreToWin, sizeof(scoreToWin));
        data[0x2D0] = damageResist;
        data[0x2D1] = healthMult;
        data[0x2D2] = healthRate;
        data[0x2D3] = shieldMult;
        data[0x2D4] = shieldRate;
        data[0x2D5] = shieldDelay;
        data[0x2D6] = headshotImmune;
        data[0x2D7] = vampirism;
        data[0x2D8] = assassinImmune;
        data[0x2DC] = weaponDamage;
        data[0x2DD] = meleeDamage;
        data[0x2DE] = grenadeRegen;
        data[0x2DF] = weaponPickup;
        data[0x2E0] = grenadeCount;
        data[0x2E1] = infiniteAmmo;
        data[0x2E2] = abilityDrop;
        data[0x2E3] = infiniteAbility;
        data[0x2E4] = abilityUsage;
        data[0x2E5] = primaryWeapon;
        data[0x2E6] = secondaryWeapon;
        data[0x2E7] = ability;
        data[0x2E8] = speed;
        data[0x2E9] = gravity;
        data[0x2EA] = vehicleUsage;
        data[0x2F0] = activeCamo;
        data[0x2F1] = waypoint;
        data[0x2F2] = visibleName;
        data[0x2F3] = aura;
        data[0x2F4] = forcedColor;
        data[0x2F8] = radarState;
        data[0x2F9] = radarRange;
        data[0x2FA] = damageIndicator;
        return true;
    }

    static bool SelfTest() {
        std::vector<uint8_t> buffer(kDecodedSize, 0);
        VariantFields in;
        in.timeLimit = 15;
        in.roundsToWin = 3;
        in.roundLimit = 2;
        in.respawnTime = 7;
        in.primaryWeapon = 5;
        in.ability = 0;
        in.speed = 2;
        in.radarState = 1;
        in.scoreToWin = 50;
        if (!in.Write(buffer.data(), buffer.size())) return false;
        VariantFields out;
        if (!out.Read(buffer.data(), buffer.size())) return false;
        return out.timeLimit == 15 && out.roundsToWin == 3 && out.roundLimit == 2 &&
               out.respawnTime == 7 && out.primaryWeapon == 5 && out.ability == 0 &&
               out.speed == 2 && out.radarState == 1 && out.scoreToWin == 50;
    }
};

} // namespace noble

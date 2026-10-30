#include "engine/PadLayouts.hpp"

#include <algorithm>

namespace noble {
namespace PadLayouts {

namespace {

// XInput button bits (XINPUT_GAMEPAD_*).
constexpr uint16_t kA = 0x1000, kB = 0x2000, kX = 0x4000, kY = 0x8000;
constexpr uint16_t kLB = 0x0100, kRB = 0x0200;
constexpr uint16_t kRS = 0x0080;

// Action / Source live in the header now (the CONTROLLER page labels its diagram from them).
constexpr Source Btn(uint16_t bit) { return Source{ bit, 0 }; }
constexpr Source Trig(int which) { return Source{ 0, which }; }

// Rows are layouts (see PadLayouts::Layout), columns the actions above. Transcribed from Halopedia's
// "Reach:Control schemes" tables (Xbox 360 manual); the MCC remaster uses the same six presets.
constexpr Source kLayouts[Count][ActionCount] = {
    // Default
    { Btn(kA), Btn(kB), Btn(kX), Btn(kY), Btn(kRB), Btn(kLB), Trig(2), Btn(kRS) },
    // Southpaw: fire/grenades swap triggers, melee/armour swap bumpers
    { Btn(kA), Trig(2), Btn(kX), Btn(kY), Btn(kLB), Btn(kRB), Trig(1), Btn(kRS) },
    // Boxer: melee on LT, grenades on RB
    { Btn(kA), Btn(kRB), Btn(kX), Btn(kY), Trig(1), Btn(kLB), Trig(2), Btn(kRS) },
    // Green Thumb: melee on the right stick click, zoom on RB
    { Btn(kA), Btn(kB), Btn(kX), Btn(kY), Btn(kRS), Btn(kLB), Trig(2), Btn(kRB) },
    // Bumper Jumper: jump LB, grenades A, action B, armour X
    { Btn(kLB), Btn(kA), Btn(kB), Btn(kY), Btn(kRB), Btn(kX), Trig(2), Btn(kRS) },
    // Recon: Halo 3 style - melee B, grenades X, action RB
    { Btn(kA), Btn(kX), Btn(kRB), Btn(kY), Btn(kB), Btn(kLB), Trig(2), Btn(kRS) },
};

// What the engine expects for each action (the default column, spelled out).
constexpr Source kDefault[ActionCount] = {
    Btn(kA), Btn(kB), Btn(kX), Btn(kY), Btn(kRB), Btn(kLB), Trig(2), Btn(kRS)
};

// Buttons the remap owns; everything else (D-pad, Back, Start, thumb clicks not used above) passes through.
constexpr uint16_t kOwned = kA | kB | kX | kY | kLB | kRB | kRS;

} // namespace

Source SourceFor(int layout, int action) {
    if (layout < 0 || layout >= Count) layout = Default;
    if (action < 0 || action >= ActionCount) action = Jump;
    return kLayouts[layout][action];
}

const char* Name(int layout) {
    switch (layout) {
    case Default: return "DEFAULT";
    case Southpaw: return "SOUTHPAW";
    case Boxer: return "BOXER";
    case GreenThumb: return "GREEN THUMB";
    case BumperJumper: return "BUMPER JUMPER";
    case Recon: return "RECON";
    default: return "DEFAULT";
    }
}

const char* Description(int layout) {
    switch (layout) {
    case Southpaw: return "Halo: Reach's Southpaw: fire on LT, grenades on RT, melee on LB and armour ability on RB.";
    case Boxer: return "Halo: Reach's Boxer: melee moves to LT and grenades to RB, so punching never leaves the trigger finger.";
    case GreenThumb: return "Halo: Reach's Green Thumb: melee on the right stick click and zoom on RB, so you can aim and punch at once.";
    case BumperJumper: return "Halo: Reach's Bumper Jumper: jump on LB, grenades on A, action on B and armour ability on X, so jumping never leaves the sticks.";
    case Recon: return "Halo: Reach's Recon (Halo 3 style): melee on B, grenades on X and action/reload on RB.";
    default: return "Halo: Reach's Default: A jump, B grenades, X action, Y swap, RB melee, LB armour ability, RT fire, LT grenades.";
    }
}

void Apply(int layout, uint16_t& buttons, uint8_t& leftTrigger, uint8_t& rightTrigger) {
    if (layout <= Default || layout >= Count) return;
    const uint16_t in = buttons;
    const uint8_t inLt = leftTrigger, inRt = rightTrigger;

    auto triggered = [&](int which) { return which == 1 ? inLt > 128 : which == 2 ? inRt > 128 : false; };
    auto triggerValue = [&](int which) -> uint8_t { return which == 1 ? inLt : which == 2 ? inRt : 0; };

    uint16_t out = static_cast<uint16_t>(in & ~kOwned);
    uint8_t outLt = 0, outRt = 0;
    for (int a = 0; a < ActionCount; ++a) {
        const Source& src = kLayouts[layout][a];
        const Source& dst = kDefault[a];
        // Is this action active, and (for analogues) with what value?
        uint8_t value = 0;
        if (src.bit) {
            if (in & src.bit) value = 255;
        } else {
            value = triggerValue(src.trigger);
        }
        if (value == 0) continue;
        if (dst.bit) {
            if (value > 128) out |= dst.bit;
        } else if (dst.trigger == 1) {
            outLt = std::max<uint8_t>(outLt, value);
        } else if (dst.trigger == 2) {
            outRt = std::max<uint8_t>(outRt, value);
        }
    }
    buttons = out;
    leftTrigger = outLt;
    rightTrigger = outRt;
}

} // namespace PadLayouts
} // namespace noble

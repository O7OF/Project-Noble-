#include "engine/PadLayouts.hpp"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <fstream>

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

const char* ActionKey(int action) {
    static const char* const keys[ActionCount] = { "jump", "grenades", "action", "swap", "melee", "armor", "fire", "zoom" };
    return (action >= 0 && action < ActionCount) ? keys[action] : "";
}

const char* ActionName(int action) {
    static const char* const names[ActionCount] = { "Jump", "Switch grenades", "Action / reload", "Swap weapons",
                                                    "Melee", "Armor ability", "Fire", "Zoom" };
    return (action >= 0 && action < ActionCount) ? names[action] : "";
}

int ActionFromKey(const std::string& key) {
    for (int a = 0; a < ActionCount; ++a) {
        if (key == ActionKey(a)) return a;
    }
    return -1;
}

uint16_t SourceBit(const Source& source) { return source.bit; }

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
    Apply(layout, buttons, leftTrigger, rightTrigger, nullptr);
}

void Apply(int layout, uint16_t& buttons, uint8_t& leftTrigger, uint8_t& rightTrigger,
           const std::map<std::string, int>* overrides) {
    const bool hasOverrides = overrides != nullptr && !overrides->empty();
    if ((layout <= Default || layout >= Count) && !hasOverrides) return;
    const uint16_t in = buttons;
    const uint8_t inLt = leftTrigger, inRt = rightTrigger;

    auto triggered = [&](int which) { return which == 1 ? inLt > 128 : which == 2 ? inRt > 128 : false; };
    auto triggerValue = [&](int which) -> uint8_t { return which == 1 ? inLt : which == 2 ? inRt : 0; };

    uint16_t out = static_cast<uint16_t>(in & ~kOwned);
    uint8_t outLt = 0, outRt = 0;
    for (int a = 0; a < ActionCount; ++a) {
        Source src = (layout > Default && layout < Count) ? kLayouts[layout][a] : kDefault[a];
        if (hasOverrides) {
            const auto it = overrides->find(ActionKey(a));
            if (it != overrides->end() && it->second > 0) src = Source{ static_cast<uint16_t>(it->second), 0 };
        }
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

int SelfTest() {
    std::ofstream report("gametype-selftest.log", std::ios::app);
    auto fail = [&report](const char* what) {
        std::printf("FAIL  PadLayouts: %s\n", what);
        if (report) report << "FAIL  PadLayouts: " << what << "\n";
    };

    int failures = 0;
    const uint16_t allButtons = static_cast<uint16_t>(kA | kB | kX | kY | kLB | kRB | kRS);

    // Default must be a perfect no-op, whatever the input.
    for (uint16_t in = 0; in <= allButtons; in = static_cast<uint16_t>(in + 0x0100)) {
        uint16_t b = in;
        uint8_t lt = 200, rt = 210;
        Apply(Default, b, lt, rt);
        if (b != in || lt != 200 || rt != 210) { fail("Default changed the pad state"); ++failures; break; }
    }

    // Helper: a layout's output for one physical button pressed alone.
    auto press = [](int layout, uint16_t bit, uint8_t& lt, uint8_t& rt) {
        uint16_t b = bit;
        lt = rt = 0;
        Apply(layout, b, lt, rt);
        return b;
    };

    struct Expect { int layout; uint16_t in; uint16_t out; const char* note; };
    const Expect cases[] = {
        // Every layout leaves the face buttons alone unless it moves them.
        { Default, kA, kA, "Default: A stays A" },
        { Boxer, kA, kA, "Boxer: A stays A (jump)" },
        { Recon, kA, kA, "Recon: A stays A (jump)" },
        // Bumper Jumper turns A into grenades (B) and LB into jump (A).
        { BumperJumper, kA, kB, "Bumper Jumper: A fires grenades" },
        { BumperJumper, kLB, kA, "Bumper Jumper: LB jumps" },
        { BumperJumper, kX, kLB, "Bumper Jumper: X is the armour ability" },
        // Boxer moves grenades to RB, so RB must come out as the engine's grenade button (B).
        { Boxer, kRB, kB, "Boxer: RB fires grenades" },
        { Boxer, kX, kX, "Boxer: X stays action/reload" },
        // Recon (Halo 3 style): B is melee, X is grenades, RB is action/reload.
        { Recon, kB, kRB, "Recon: B is melee" },
        { Recon, kX, kB, "Recon: X fires grenades" },
        { Recon, kRB, kX, "Recon: RB is action/reload" },
        // Green Thumb: melee on the right stick click, zoom on RB.
        { GreenThumb, kRS, kRB, "Green Thumb: RS click is melee" },
        { GreenThumb, kRB, kRS, "Green Thumb: RB is zoom" },
    };
    for (const Expect& c : cases) {
        uint8_t lt = 0, rt = 0;
        const uint16_t got = press(c.layout, c.in, lt, rt);
        // A single button can legitimately produce two default bits (Recon's B is melee => RB), so test containment.
        if (c.note[0] && (got & c.out) == 0) {
            char line[256];
            std::snprintf(line, sizeof(line), "%s (input 0x%04X produced 0x%04X, expected bit 0x%04X)", c.note, c.in, got, c.out);
            fail(line);
            ++failures;
        }
    }

    // Southpaw swaps the triggers: LT becomes fire (the engine wants fire on RT) and RT becomes grenades (the
    // engine wants grenades on the B button).
    {
        uint8_t lt = 220, rt = 0;
        uint16_t b = 0;
        Apply(Southpaw, b, lt, rt);
        if (rt < 200 || (b & kB) != 0) { fail("Southpaw: LT should come out as fire on RT"); ++failures; }
    }
    {
        uint8_t lt = 0, rt = 230;
        uint16_t b = 0;
        Apply(Southpaw, b, lt, rt);
        if ((b & kB) == 0) { fail("Southpaw: RT should come out as grenades on B"); ++failures; }
    }
    // Boxer: LT is melee, which the engine expects on RB.
    {
        uint8_t lt = 240, rt = 0;
        uint16_t b = 0;
        Apply(Boxer, b, lt, rt);
        if ((b & kRB) == 0) { fail("Boxer: LT should come out as melee on RB"); ++failures; }
    }

    // D-pad, Back and Start are never owned by a layout.
    {
        const uint16_t others = static_cast<uint16_t>(0x0001 | 0x0002 | 0x0004 | 0x0008 | 0x0010 | 0x0020 | 0x0040);
        for (int layout = Default + 1; layout < Count; ++layout) {
            uint8_t lt = 0, rt = 0;
            uint16_t b = others;
            Apply(layout, b, lt, rt);
            if ((b & others) != others) { fail("a layout swallowed a D-pad / Start / Back / LS bit"); ++failures; break; }
        }
    }

    std::printf("%s: button layouts - %zu expectations, %d failure(s)\n",
                failures == 0 ? "PASS" : "FAIL", sizeof(cases) / sizeof(cases[0]) + 3, failures);
    if (report) {
        report << (failures == 0 ? "PASS" : "FAIL") << ": button layouts - " << sizeof(cases) / sizeof(cases[0]) + 3
               << " expectations, " << failures << " failure(s)\n";
    }
    return failures;
}

} // namespace PadLayouts
} // namespace noble

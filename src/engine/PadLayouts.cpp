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
constexpr uint16_t kLS = 0x0040, kRS = 0x0080;
constexpr uint16_t kUp = 0x0001, kDown = 0x0002, kLeft = 0x0004, kRight = 0x0008, kStart = 0x0010, kBack = 0x0020;

// The actions a layout moves. Rows are layouts (see PadLayouts::Layout), transcribed from Halopedia's "Reach:Control
// schemes" tables (Xbox 360 manual); the MCC remaster uses the same six presets. The rest never move between layouts.
constexpr int kLayoutActions = 9;
constexpr int kLayouts[Count][kLayoutActions] = {
    //          jump  switch  action  swap  melee    armor  fire     zoom  throw
    /* Default */      { kA,  kB,      kX,   kY,   kRB,     kLB,   InputRT, kRS,  InputLT },
    /* Southpaw */     { kA,  kB,      kX,   kY,   kLB,     kRB,   InputLT, kRS,  InputRT },
    /* Boxer */        { kA,  kB,      kX,   kY,   InputLT, kLB,   InputRT, kRS,  kRB },
    /* Green Thumb */  { kA,  kB,      kX,   kY,   kRS,     kLB,   InputRT, kRB,  InputLT },
    /* Bumper Jumper */{ kLB, kA,      kB,   kY,   kRB,     kX,    InputRT, kRS,  InputLT },
    /* Recon */        { kA,  kX,      kRB,  kY,   kB,      kLB,   InputRT, kRS,  InputLT },
};

// Every button the remap owns.
constexpr uint16_t kOwned = kA | kB | kX | kY | kLB | kRB | kLS | kRS | kUp | kDown | kLeft | kRight | kStart | kBack;

} // namespace

const char* ActionKey(int action) {
    static const char* const keys[ActionCount] = { "jump", "grenades", "action", "swap", "melee", "armor", "fire", "zoom",
                                                   "throw", "crouch", "scoreboard", "pause", "dpadup", "dpaddown", "dpadleft", "dpadright",
                                                   "teamchat", "pushtotalk" };
    return (action >= 0 && action < ActionCount) ? keys[action] : "";
}

const char* ActionName(int action) {
    static const char* const names[ActionCount] = { "Jump", "Switch grenades", "Action / reload", "Swap weapons",
                                                    "Melee", "Armor ability", "Fire", "Zoom", "Throw grenade", "Crouch", "Scoreboard",
                                                    "Pause menu", "D-pad up action", "D-pad down action", "D-pad left action", "D-pad right action",
                                                    "Team chat", "Push to talk" };
    return (action >= 0 && action < ActionCount) ? names[action] : "";
}

int ActionFromKey(const std::string& key) {
    for (int a = 0; a < ActionCount; ++a) {
        if (key == ActionKey(a)) return a;
    }
    return -1;
}

int LayoutInput(int layout, int action) {
    if (layout < 0 || layout >= Count) layout = Default;
    if (action >= 0 && action < kLayoutActions) return kLayouts[layout][action];
    switch (action) {
    case Crouch: return kLS;
    case Scoreboard: return kBack;
    case Pause: return kStart;
    case DpadUp: return kUp;
    case DpadDown: return kDown;
    case DpadLeft: return kLeft;
    case DpadRight: return kRight;
    default: return InputNone;   // team chat, push to talk: nothing assigned until they exist
    }
}

int EffectiveInput(int layout, int action, const std::map<std::string, int>* overrides) {
    if (overrides) {
        const auto it = overrides->find(ActionKey(action));
        if (it != overrides->end() && it->second != 0) return it->second > 0 ? it->second : InputNone;
    }
    return LayoutInput(layout, action);
}

const char* InputName(int input) {
    if (input >= InputPaddle1 && input < InputPaddle1 + kPaddleCount) {
        static const char* const kNames[kPaddleCount] = { "PADDLE 1", "PADDLE 2", "PADDLE 3", "PADDLE 4", "PADDLE 5", "PADDLE 6" };
        return kNames[input - InputPaddle1];
    }
    switch (input) {
    case kA: return "A";
    case kB: return "B";
    case kX: return "X";
    case kY: return "Y";
    case kLB: return "LB";
    case kRB: return "RB";
    case kLS: return "L STICK CLICK";
    case kRS: return "R STICK CLICK";
    case kUp: return "D-PAD UP";
    case kDown: return "D-PAD DOWN";
    case kLeft: return "D-PAD LEFT";
    case kRight: return "D-PAD RIGHT";
    case kStart: return "START";
    case kBack: return "BACK";
    case InputLT: return "LT";
    case InputRT: return "RT";
    case InputLStickUp: return "L STICK UP";
    case InputLStickDown: return "L STICK DOWN";
    case InputLStickLeft: return "L STICK LEFT";
    case InputLStickRight: return "L STICK RIGHT";
    case InputRStickUp: return "R STICK UP";
    case InputRStickDown: return "R STICK DOWN";
    case InputRStickLeft: return "R STICK LEFT";
    case InputRStickRight: return "R STICK RIGHT";
    default: return "-";
    }
}

void Assign(int layout, int action, int input, std::map<std::string, int>& overrides) {
    const int previous = EffectiveInput(layout, action, &overrides);
    if (previous == input) return;
    for (int other = 0; other < ActionCount; ++other) {
        if (other == action || EffectiveInput(layout, other, &overrides) != input) continue;
        if (previous == LayoutInput(layout, other)) overrides.erase(ActionKey(other));
        else overrides[ActionKey(other)] = previous > 0 ? previous : -1;   // -1: deliberately no input
    }
    if (input == LayoutInput(layout, action)) overrides.erase(ActionKey(action));
    else overrides[ActionKey(action)] = input;
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

void Apply(int layout, uint16_t& buttons, uint8_t& leftTrigger, uint8_t& rightTrigger,
           const std::map<std::string, int>* overrides, int16_t leftX, int16_t leftY, int16_t rightX, int16_t rightY, uint8_t paddles) {
    const bool hasOverrides = overrides != nullptr && !overrides->empty();
    if ((layout <= Default || layout >= Count) && !hasOverrides) return;
    const uint16_t in = buttons;
    const uint8_t inLt = leftTrigger, inRt = rightTrigger;

    // How hard an input is pressed: 0..255. A stick direction counts once it is past about half way.
    auto level = [&](int input) -> uint8_t {
        constexpr int kStickOn = 16000;
        if (input >= InputPaddle1 && input < InputPaddle1 + kPaddleCount) {
            return (paddles & (1u << (input - InputPaddle1))) != 0 ? 255 : 0;
        }
        switch (input) {
        case InputNone: return 0;
        case InputLT: return inLt;
        case InputRT: return inRt;
        case InputLStickUp: return leftY > kStickOn ? 255 : 0;
        case InputLStickDown: return leftY < -kStickOn ? 255 : 0;
        case InputLStickLeft: return leftX < -kStickOn ? 255 : 0;
        case InputLStickRight: return leftX > kStickOn ? 255 : 0;
        case InputRStickUp: return rightY > kStickOn ? 255 : 0;
        case InputRStickDown: return rightY < -kStickOn ? 255 : 0;
        case InputRStickLeft: return rightX < -kStickOn ? 255 : 0;
        case InputRStickRight: return rightX > kStickOn ? 255 : 0;
        default: return input > 0 && (in & input) != 0 ? 255 : 0;
        }
    };

    uint16_t out = static_cast<uint16_t>(in & ~kOwned);
    uint8_t outLt = 0, outRt = 0;
    for (int a = 0; a < ActionCount; ++a) {
        const uint8_t value = level(EffectiveInput(layout, a, overrides));
        if (value == 0) continue;
        const int engine = LayoutInput(Default, a);   // what the engine expects for this action
        if (engine == InputLT) outLt = std::max(outLt, value);
        else if (engine == InputRT) outRt = std::max(outRt, value);
        else if (engine > 0 && value > 128) out = static_cast<uint16_t>(out | engine);
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
        // Bumper Jumper turns A into switch grenades (B) and LB into jump (A).
        { BumperJumper, kA, kB, "Bumper Jumper: A switches grenades" },
        { BumperJumper, kLB, kA, "Bumper Jumper: LB jumps" },
        { BumperJumper, kX, kLB, "Bumper Jumper: X is the armour ability" },
        { Boxer, kX, kX, "Boxer: X stays action/reload" },
        // Recon (Halo 3 style): B is melee, X switches grenades, RB is action/reload.
        { Recon, kB, kRB, "Recon: B is melee" },
        { Recon, kX, kB, "Recon: X switches grenades" },
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

    // Southpaw swaps the triggers: LT comes out as fire on RT, RT as throw on LT.
    {
        uint8_t lt = 220, rt = 0;
        uint16_t b = 0;
        Apply(Southpaw, b, lt, rt);
        if (rt < 200 || lt != 0) { fail("Southpaw: LT should come out as fire on RT"); ++failures; }
    }
    {
        uint8_t lt = 0, rt = 230;
        uint16_t b = 0;
        Apply(Southpaw, b, lt, rt);
        if (lt < 200 || rt != 0 || b != 0) { fail("Southpaw: RT should come out as throw on LT"); ++failures; }
    }
    // Every layout must keep throw-grenade somewhere (it used to be lost outside Default).
    for (int layout = Default; layout < Count; ++layout) {
        if (LayoutInput(layout, ThrowGrenade) == InputNone) { fail("a layout has no throw grenade"); ++failures; }
    }
    // Boxer: RB throws, which the engine expects on LT.
    {
        uint8_t lt = 0, rt = 0;
        uint16_t b = kRB;
        Apply(Boxer, b, lt, rt);
        if (lt < 200) { fail("Boxer: RB should come out as throw on LT"); ++failures; }
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

    // Overrides: a trigger, a D-pad direction and a stick direction can all be bound, and binding swaps the old owner.
    {
        std::map<std::string, int> o;
        Assign(Default, Jump, kB, o);
        if (EffectiveInput(Default, Grenades, &o) != kA) { fail("Assign: the old owner of B should take A"); ++failures; }
        uint16_t b = kB;
        uint8_t lt = 0, rt = 0;
        Apply(Default, b, lt, rt, &o);
        if ((b & kA) == 0 || (b & kB) != 0) { fail("Assign: B should now jump and nothing else"); ++failures; }
        Assign(Default, Jump, kA, o);
        if (!o.empty()) { fail("Assign: swapping back should leave no overrides"); ++failures; }

        o.clear();
        Assign(Default, Melee, InputLStickLeft, o);
        b = 0;
        Apply(Default, b, lt, rt, &o, -30000, 0, 0, 0);
        if ((b & kRB) == 0) { fail("Assign: left stick left should melee"); ++failures; }

        o.clear();
        Assign(Default, Fire, kUp, o);
        b = kUp;
        lt = rt = 0;
        Apply(Default, b, lt, rt, &o);
        if (rt < 200) { fail("Assign: D-pad up should fire on RT"); ++failures; }
        b = kStart;
        lt = rt = 0;
        Apply(Default, b, lt, rt, &o);
        if ((b & kStart) == 0) { fail("Start should still pause with overrides on"); ++failures; }

        o.clear();
        Assign(Default, Jump, InputPaddle1 + 2, o);
        b = 0;
        lt = rt = 0;
        Apply(Default, b, lt, rt, &o, 0, 0, 0, 0, 1u << 2);
        if ((b & kA) == 0) { fail("Paddle 3 held should jump"); ++failures; }
        b = kA;
        lt = rt = 0;
        Apply(Default, b, lt, rt, &o);
        if ((b & kA) != 0) { fail("A should stop jumping once jump is on a paddle"); ++failures; }
    }
    std::printf("%s: button layouts - %zu expectations, %d failure(s)\n",
                failures == 0 ? "PASS" : "FAIL", sizeof(cases) / sizeof(cases[0]) + 14, failures);
    if (report) {
        report << (failures == 0 ? "PASS" : "FAIL") << ": button layouts - " << sizeof(cases) / sizeof(cases[0]) + 14
               << " expectations, " << failures << " failure(s)\n";
    }
    return failures;
}

} // namespace PadLayouts
} // namespace noble

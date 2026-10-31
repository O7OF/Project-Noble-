#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace noble {

// Halo: Reach's own button layouts (Xbox 360 manual / Halopedia "Reach:Control schemes"), applied by Noble to the
// pad state before the engine sees it. The engine has no layout of its own outside MCC, so this is a Noble-side
// remap of which physical input fires which action.
//
// Actions and their DEFAULT inputs (what the engine expects):
//   Jump = A, Switch grenades = B, Action/reload = X, Swap weapons = Y, Melee = RB, Armor ability = LB,
//   Fire = RT (analogue), Throw grenade = LT (analogue), Zoom = RS click, Crouch = LS click, Scoreboard = Back,
//   Pause = Start, and the four D-pad directions.
namespace PadLayouts {

enum Layout {
    Default = 0,   // A jump, B switch grenades, X action, Y swap, RB melee, LB armor, RT fire, LT throw
    Southpaw,      // triggers and bumpers swapped (fire LT, throw RT, melee LB, armor RB)
    Boxer,         // melee on LT, throw on RB
    GreenThumb,    // melee on RS click, zoom on RB
    BumperJumper,  // jump LB, switch grenades A, action B, armor X
    Recon,         // Halo 3 style: melee B, switch grenades X, action RB
    Count
};

// Team chat and push-to-talk can be assigned, but nothing listens to them until chat and voice exist.
enum Action {
    Jump = 0, Grenades, ActionReload, SwapWeapons, Melee, ArmorAbility, Fire, Zoom,
    ThrowGrenade, Crouch, Scoreboard, Pause, DpadUp, DpadDown, DpadLeft, DpadRight, TeamChat, PushToTalk,
    ActionCount
};

// A physical input, as a bit mask: the low 16 bits are XInput button bits, the rest are the inputs XInput reports as
// analogue values. Saved in settings.json, so the values must not change.
enum InputBit : int {
    InputNone = 0,
    InputLT = 0x010000, InputRT = 0x020000,
    InputLStickUp = 0x040000, InputLStickDown = 0x080000, InputLStickLeft = 0x100000, InputLStickRight = 0x200000,
    InputRStickUp = 0x400000, InputRStickDown = 0x800000, InputRStickLeft = 0x1000000, InputRStickRight = 0x2000000,
    // Paddles reach Noble as keyboard keys or mouse buttons (SettingsData::padPaddleKeys). InputPaddle1 + n for paddle n.
    InputPaddle1 = 0x40000001,
};
constexpr int kPaddleCount = 6;

// The input one action uses in `layout`, before any player override.
int LayoutInput(int layout, int action);
// The input it really uses: the player's override when there is one, else the layout's.
int EffectiveInput(int layout, int action, const std::map<std::string, int>* overrides);
const char* InputName(int input);      // "A", "LT", "L STICK LEFT", ..., "-" when unbound

// Stable key and player-facing name for each action, for settings.json and the bindings list.
const char* ActionKey(int action);     // "jump", "grenades", "action", "swap", ...
const char* ActionName(int action);    // "Jump", "Switch grenades", ...
int ActionFromKey(const std::string& key);   // -1 when unknown

// Assigns `input` to `action`. An action already on that input is given `action`'s old input in exchange, so nothing
// ends up with two jobs on one button or with no button.
void Assign(int layout, int action, int input, std::map<std::string, int>& overrides);

// Rewrites a raw XInput pad state into the engine's default mapping for this layout and these overrides. `layout` out of
// range (or Default) without overrides leaves everything untouched. `paddles` has bit n set while paddle n is held. The stick values are only read for overrides bound to a
// stick direction; the sticks themselves keep moving and looking.
void Apply(int layout, uint16_t& buttons, uint8_t& leftTrigger, uint8_t& rightTrigger,
           const std::map<std::string, int>* overrides = nullptr,
           int16_t leftX = 0, int16_t leftY = 0, int16_t rightX = 0, int16_t rightY = 0, uint8_t paddles = 0);

// Display name of a layout ("DEFAULT", "SOUTHPAW", ...). Never null.
const char* Name(int layout);

// One-line description for the Settings details column. Never null.
const char* Description(int layout);

// Headless check of the layout tables (run by `-test-map-gametypes`). Returns the number of failures.
int SelfTest();

} // namespace PadLayouts
} // namespace noble

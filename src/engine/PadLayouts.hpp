#pragma once

#include <cstdint>

namespace noble {

// Halo: Reach's own button layouts (Xbox 360 manual / Halopedia "Reach:Control schemes"), applied by Noble to the
// pad state before the engine sees it. The engine has no layout of its own outside MCC, so this is a Noble-side
// remap of which physical button fires which action.
//
// Actions and their DEFAULT buttons (what the engine expects):
//   Jump = A, Switch grenades = B, Action/reload = X, Swap weapons = Y,
//   Melee = RB, Armor ability = LB, Fire = RT (analogue), Zoom = RS click.
// The D-pad, Back, Start and the sticks' movement/look axes are the same in every layout.
namespace PadLayouts {

enum Layout {
    Default = 0,   // A jump, B grenades, X action, Y swap, RB melee, LB armor, RT fire, LT grenades
    Southpaw,      // triggers and bumpers swapped (fire LT, grenades RT, melee LB, armor RB)
    Boxer,         // melee on LT, grenades on RB
    GreenThumb,    // melee on RS click, zoom on RB
    BumperJumper,  // jump LB, grenades A, action B, armor X
    Recon,         // Halo 3 style: melee B, grenades X, action RB
    Count
};

// The actions a layout assigns (and the engine expects in this order).
enum Action { Jump = 0, Grenades, ActionReload, SwapWeapons, Melee, ArmorAbility, Fire, Zoom, ActionCount };

// Where an action's input comes from in a layout: a button bit, or one of the analogue triggers.
struct Source {
    uint16_t bit = 0;   // XInput button, 0 when the source is a trigger
    int trigger = 0;    // 0 = none, 1 = left trigger, 2 = right trigger
};

// The source for one action in `layout` (clamped). Used by the CONTROLLER page to label its diagram.
Source SourceFor(int layout, int action);

// Display name of a layout ("DEFAULT", "SOUTHPAW", ...). Never null.
const char* Name(int layout);

// One-line description for the Settings details column. Never null.
const char* Description(int layout);

// Rewrites a raw XInput pad state (button bits + trigger values) into the engine's default mapping for this
// layout. `layout` out of range (or Default) leaves everything untouched. Non-action bits are passed through.
void Apply(int layout, uint16_t& buttons, uint8_t& leftTrigger, uint8_t& rightTrigger);

} // namespace PadLayouts
} // namespace noble

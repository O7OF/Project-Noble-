#pragma once

#include <imgui.h>

namespace noble {

// Halo: Reach's eight multiplayer teams, in engine order (team 1 = Red ... team 8 = Pink). The lobby's team
// dividers, the team strip on a player row and the match scoreboard all read this one table so a team always
// looks the same wherever it is drawn.
struct TeamStyle {
    const char* name;       // "RED TEAM"
    const char* shortName;  // "RED"
    ImU32 header;           // divider bar / team header
    ImU32 rowBase;          // first roster row tint
    ImU32 rowAlt;           // second roster row tint
    ImU32 strip;            // the strip on a player row
};

inline constexpr int kTeamCount = 8;

inline constexpr TeamStyle kTeamStyles[kTeamCount] = {
    { "RED TEAM",    "RED",    ImU32(IM_COL32(195, 42, 42, 245)),  ImU32(IM_COL32(145, 30, 30, 150)),  ImU32(IM_COL32(130, 25, 25, 150)),  ImU32(IM_COL32(200, 45, 45, 255)) },
    { "BLUE TEAM",   "BLUE",   ImU32(IM_COL32(38, 92, 185, 245)),  ImU32(IM_COL32(28, 62, 145, 150)),  ImU32(IM_COL32(24, 52, 130, 150)),  ImU32(IM_COL32(45, 100, 200, 255)) },
    { "GREEN TEAM",  "GREEN",  ImU32(IM_COL32(38, 160, 68, 245)),  ImU32(IM_COL32(28, 120, 52, 150)),  ImU32(IM_COL32(24, 105, 45, 150)),  ImU32(IM_COL32(45, 175, 75, 255)) },
    { "ORANGE TEAM", "ORANGE", ImU32(IM_COL32(220, 115, 28, 245)), ImU32(IM_COL32(165, 85, 20, 150)),  ImU32(IM_COL32(145, 75, 18, 150)),  ImU32(IM_COL32(235, 125, 35, 255)) },
    { "PURPLE TEAM", "PURPLE", ImU32(IM_COL32(150, 48, 195, 245)), ImU32(IM_COL32(110, 35, 145, 150)), ImU32(IM_COL32(95, 30, 130, 150)),  ImU32(IM_COL32(160, 55, 210, 255)) },
    { "GOLD TEAM",   "GOLD",   ImU32(IM_COL32(215, 180, 30, 245)), ImU32(IM_COL32(160, 135, 22, 150)), ImU32(IM_COL32(140, 118, 18, 150)), ImU32(IM_COL32(230, 195, 40, 255)) },
    { "BROWN TEAM",  "BROWN",  ImU32(IM_COL32(140, 85, 45, 245)),  ImU32(IM_COL32(105, 62, 32, 150)),  ImU32(IM_COL32(90, 54, 28, 150)),   ImU32(IM_COL32(155, 95, 50, 255)) },
    { "PINK TEAM",   "PINK",   ImU32(IM_COL32(225, 80, 150, 245)), ImU32(IM_COL32(165, 58, 110, 150)), ImU32(IM_COL32(145, 50, 96, 150)),  ImU32(IM_COL32(240, 90, 160, 255)) },
};

// Team numbers are 1-based everywhere (0 = no team / free for all).
inline const TeamStyle& TeamStyleFor(int team) {
    return kTeamStyles[(team >= 1 && team <= kTeamCount) ? team - 1 : 0];
}

} // namespace noble

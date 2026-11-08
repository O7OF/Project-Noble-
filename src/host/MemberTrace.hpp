#pragma once

#include <windows.h>

// C12 (128 players), dev builds only: records every haloreach.dll instruction that touches the player / peer
// slot arrays of the engine's four network session objects (static array at RVA 0x2B07418, 4 x 0x61668 bytes,
// constructor 0x3CF864). Opt-in with -trace-members (NOBLE_TRACE_MEMBERS=1). Output: "MEMBER|..." log lines.
namespace noble::membertrace {
void Install(HMODULE engine);
}

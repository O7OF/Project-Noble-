#pragma once
// Experimental AI pool sizes (LIMITS-TODO C7). Opt-in, extended profiles only.
//
// Every engine data array (actor, prop, squad, command scripts, ...) is created by one function,
// data_array_new(name, count, record size, alignment, allocator) at haloreach+0x12FCC. Hooking it lets Noble
// raise a pool by name without rewriting the engine code that passes the count. The pools live in the
// game-state banks, which V16 already enlarged (main bank 39.8 MB; all pools together use about 16 MB).
//
// Enable with "-ai-actors <count>" (or the NOBLE_AI_ACTORS environment variable, which the relaunched
// match process inherits). The actor pool becomes <count> (stock 96) and the pools actors use are scaled
// with it: prop and prop_ref by count/96, command scripts and object list headers to at least 256,
// dynamic firing points to 64, swarms to 128. Every change is logged ("AI limits: actor 96 -> 256").
// Ignored on the Stock profile, whose game-state banks are too small.
#include <windows.h>

namespace noble::pools {

void InstallResize(HMODULE engine, bool extendedProfile);

} // namespace noble::pools

#pragma once
// Stack guard for the engine's recursive collision-hierarchy walk.
//
// `objects near this object` queries (haloreach+0x1D6380 -> 0x150BC0) walk an object's collision
// hierarchy (the 0x1C-byte "collision hierarchy node header" pool) recursively: every node keeps its
// frame in 0x70 bytes and recurses into its two children (haloreach+0x150A30, recursive call at
// 0x150B8C). The walk trusts the hierarchy to be acyclic and -1 terminated.
//
// When the hierarchy holds a stale child handle - a node that was freed/reinitialised while a parent
// still points at it - the freed node is all zeroes, and a zeroed node's child fields read as node 0
// instead of -1. The walk then loops forever (0x150A30 -> 0x150B8C -> 0x150A30 ...) until the thread's
// stack is exhausted: 0xC00000FD inside haloreach.dll a few seconds into the menu or a match, with the
// faulting stack nothing but 0x150B91 frames (9,130 of them before the guard). Observed with the
// extended-limit object pool (2,048 -> 32,767), which changes which object slots the menu's Armory
// rebuild recycles.
//
// The guard bounds the recursion (depth + remaining stack) and returns "no objects found" for the
// subtree it refuses to enter, so a damaged hierarchy costs a few nearby objects instead of the process.
#include <windows.h>

namespace noble::guards {

void InstallCollisionWalkGuard(HMODULE engine);

} // namespace noble::guards

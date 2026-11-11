#pragma once

#include <windows.h>

namespace noble {

// Hooks the engine's object_new (RVA 0x46D6E0), which Forge calls for every object placed in the editor, and records
// the placement data (tag index, position, facing). This is where a placed object exists before any variant record is
// written, so it is the capture point for building variants out of Forge edits.
void InstallForgePlacementWatch(HMODULE reachModule);

} // namespace noble

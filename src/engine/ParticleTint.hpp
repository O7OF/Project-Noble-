#pragma once

#include <d3d11.h>
#include <string>

namespace noble::ParticleTint {

// Dev prototype (-armor-fx-runtime-tint r,g,b or hex):
// Replaces the 256x16 armour-flame palette texture (identified at runtime by its
// dimensions/format) with a runtime-generated colour palette. This is the first
// step of the runtime-colour path: no map rebuild, works on stock maps.
// Per-player colour is not implemented yet - every particle system using the
// palette in this process sees the tint.
void Install(ID3D11Device* device, ID3D11DeviceContext* context);
// Called from the DXGI Present hook: obtains the device/immediate context that owns the presenting
// swapchain (the engine may have its own context, separate from the launcher's) and hooks it.
void HookFromSwapChain(struct IDXGISwapChain* swapChain);
void Shutdown();

} // namespace noble::ParticleTint

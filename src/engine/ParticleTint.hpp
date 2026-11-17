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
void Shutdown();

} // namespace noble::ParticleTint

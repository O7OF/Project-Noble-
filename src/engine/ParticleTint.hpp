#pragma once

#include <d3d11.h>
#include <cstdint>
#include <string>

namespace noble::ParticleTint {

// Runtime recolouring of the stock 256x16 flame palette (slot 2, fmt 91).
// Supports all 30 authentic Reach primary flame variants across the Armory
// preview and multiplayer matches (both host and non-host peer bipeds).
void Install(ID3D11Device* device, ID3D11DeviceContext* context);
void HookFromSwapChain(struct IDXGISwapChain* swapChain);
void HookFromContext(struct ID3D11DeviceContext* context);

// Local player's selected flame variant index (0..29, or -1 for none/stock)
void SetVariantIndex(int variantIndex);
int GetVariantIndex();

// Multiplayer per-biped flame registration
void RegisterBipedFlame(uint32_t bipedDatum, int variantIndex);
void UnregisterBiped(uint32_t bipedDatum);
void NoteRenderedBiped(uint32_t bipedDatum);
int GetBipedFlameVariant(uint32_t bipedDatum);

void Shutdown();

} // namespace noble::ParticleTint

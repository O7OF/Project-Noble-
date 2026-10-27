#include "RasterizerAtomHooks.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"

namespace noble {

static uintptr_t s_haloReachBase = 0;

// Dummy original function pointers required by MinHook
static void* s_orig_0x2743A4 = nullptr;
static void* s_orig_0x274488 = nullptr;
static void* s_orig_0x274564 = nullptr;
static void* s_orig_0x2745E8 = nullptr;
static void* s_orig_0x274670 = nullptr;
static void* s_orig_0x2746F8 = nullptr;
static void* s_orig_0x274758 = nullptr;
static void* s_orig_0x2747AC = nullptr;
static void* s_orig_0x274800 = nullptr;

// 1. RVA 0x2743A4: UpdatePool
static void __fastcall Hook_0x2743A4(uint32_t index) {
    if (!s_haloReachBase) return;
    uintptr_t base = s_haloReachBase;
    uint32_t r10d = *reinterpret_cast<const uint32_t*>(base + 0xB44458);
    uintptr_t rbx = base + 0xC8E4B0;
    uint32_t r11d = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);

    *reinterpret_cast<uint32_t*>(base + 0x4E38A24) = index;

    uint8_t flagResult = 0;
    if (index != 0) {
        int64_t rax = static_cast<int32_t>(index);
        uintptr_t r9 = static_cast<uintptr_t>(rax * 0x70);
        uint32_t eax = 0;
        uint32_t r8d = *reinterpret_cast<const uint32_t*>(rbx + r9 + 0x10);
        if (r8d & 0x100) {
            eax = *reinterpret_cast<const uint32_t*>(rbx + r9 + 0x68);
        }
        if (r8d & 0x08) {
            eax += eax;
            if (index == 0x0C) {
                eax += r10d;
            }
        }
        if (r8d & 0x20) {
            eax = r11d + eax * 4;
        }
        int64_t atomOffset = static_cast<int64_t>(static_cast<int32_t>(eax)) * 0x88;
        uintptr_t atomPtr = *reinterpret_cast<const uintptr_t*>(rbx + r9 + 0x60);
        if (atomPtr != 0) {
            uintptr_t r8 = atomOffset + atomPtr;
            uint32_t val44 = *reinterpret_cast<const uint32_t*>(r8 + 0x44);
            if (val44 == 0x14 || val44 == 0x28) {
                flagResult = 1;
            }
        }
    }
    *reinterpret_cast<uint8_t*>(base + 0xB43ADC) = flagResult;

    // Part 2: Read atom pointer & update cache
    int64_t rax2 = static_cast<int32_t>(index);
    uintptr_t r8_2 = static_cast<uintptr_t>(rax2 * 0x70);
    uint32_t eax2 = *reinterpret_cast<const uint32_t*>(rbx + r8_2 + 0x10);
    uint32_t edx2 = 0;
    if (eax2 & 0x100) {
        edx2 = *reinterpret_cast<const uint32_t*>(rbx + r8_2 + 0x68);
    }
    if (eax2 & 0x08) {
        edx2 += edx2;
        if (index == 0x0C) {
            edx2 += r10d;
        }
    }
    if (eax2 & 0x20) {
        edx2 = r11d + edx2 * 4;
    }
    int64_t rcx2 = static_cast<int64_t>(static_cast<int32_t>(edx2)) * 0x88;
    const void* atomArray2 = *reinterpret_cast<const void* const*>(rbx + r8_2 + 0x60);

    const void* rdx_val = nullptr;
    if (atomArray2 != nullptr) {
        rdx_val = *reinterpret_cast<const void* const*>(reinterpret_cast<uintptr_t>(atomArray2) + rcx2);
    }

    const void** pCached = reinterpret_cast<const void**>(base + 0x4E38A28);
    if (*pCached != rdx_val) {
        *reinterpret_cast<uint32_t*>(base + 0x4E38A20) |= 0x70;
        *pCached = rdx_val;
    }
}

// 2. RVA 0x274488: BindAtom
static void __fastcall Hook_0x274488(uint32_t slot, uint32_t index) {
    if (!s_haloReachBase) return;
    uintptr_t base = s_haloReachBase;
    int64_t r11 = static_cast<int32_t>(slot);
    int64_t rax = static_cast<int32_t>(index);
    uintptr_t r10 = static_cast<uintptr_t>(rax * 0x70);
    uint32_t r8d = 0;

    uint32_t flags = *reinterpret_cast<const uint32_t*>(base + 0xC8E4C0 + r10);
    *reinterpret_cast<uint32_t*>(base + 0x4E38A10 + (r11 * 4)) = index;

    if (flags & 0x100) {
        r8d = *reinterpret_cast<const uint32_t*>(base + 0xC8E518 + r10);
    }
    if (flags & 0x08) {
        r8d += r8d;
        if (index == 0x0C) {
            r8d += *reinterpret_cast<const uint32_t*>(base + 0xB44458);
        }
    }
    if (flags & 0x20) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);
        r8d = val + r8d * 4;
    }

    int64_t atomOffset = static_cast<int64_t>(static_cast<int32_t>(r8d)) * 0x88;
    const void* atomArray = *reinterpret_cast<const void* const*>(base + 0xC8E510 + r10);

    const void* rdx = nullptr;
    if (atomArray) {
        rdx = *reinterpret_cast<const void* const*>(reinterpret_cast<uintptr_t>(atomArray) + atomOffset + 8);
    }

    const void** slotTarget = reinterpret_cast<const void**>(base + 0xCA02E0 + (r11 * 8));
    if (*slotTarget != rdx) {
        *slotTarget = rdx;
        uint32_t state = *reinterpret_cast<const uint32_t*>(base + 0x4E38A20);
        state |= 0x60;
        state |= (1u << (r11 & 0x1F));
        *reinterpret_cast<uint32_t*>(base + 0x4E38A20) = state;
    }
}

// 3. RVA 0x274564: BindAtomExt1
static void __fastcall Hook_0x274564(uint32_t slot, uint32_t index) {
    if (!s_haloReachBase) return;
    uintptr_t base = s_haloReachBase;
    int64_t rax = static_cast<int32_t>(index);
    int64_t r10 = static_cast<int32_t>(slot);
    uintptr_t r9 = static_cast<uintptr_t>(rax * 0x70);
    uint32_t r8d = 0;

    uint32_t flags = *reinterpret_cast<const uint32_t*>(base + 0xC8E4C0 + r9);
    if (flags & 0x100) {
        r8d = *reinterpret_cast<const uint32_t*>(base + 0xC8E518 + r9);
    }
    if (flags & 0x08) {
        r8d += r8d;
        if (index == 0x0C) {
            r8d += *reinterpret_cast<const uint32_t*>(base + 0xB44458);
        }
    }
    if (flags & 0x20) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);
        r8d = val + r8d * 4;
    }

    int64_t rcx = static_cast<int64_t>(static_cast<int32_t>(r8d)) * 0x88;
    const void* atomArray = *reinterpret_cast<const void* const*>(base + 0xC8E510 + r9);
    const void* rdx = nullptr;
    if (atomArray) {
        rdx = *reinterpret_cast<const void* const*>(reinterpret_cast<uintptr_t>(atomArray) + rcx + 0x18);
    }

    *reinterpret_cast<uint64_t*>(base + 0xB44470 + (r10 * 8)) = ~0ULL;
    *reinterpret_cast<uint32_t*>(base + 0xCA02D8) |= (1u << (r10 & 0x1F));
    *reinterpret_cast<const void**>(base + 0xCA03A0 + (r10 * 8)) = rdx;
}

// 4. RVA 0x2745E8: BindAtomExt2
static void __fastcall Hook_0x2745E8(uint32_t slot, uint32_t index) {
    if (!s_haloReachBase) return;
    uintptr_t base = s_haloReachBase;
    int64_t rax = static_cast<int32_t>(index);
    int32_t r10d = static_cast<int32_t>(slot);
    uintptr_t r9 = static_cast<uintptr_t>(rax * 0x70);
    uint32_t ecx = 0;

    uint32_t flags = *reinterpret_cast<const uint32_t*>(base + 0xC8E4C0 + r9);
    if (flags & 0x100) {
        ecx = *reinterpret_cast<const uint32_t*>(base + 0xC8E518 + r9);
    }
    if (flags & 0x08) {
        ecx += ecx;
        if (index == 0x0C) {
            ecx += *reinterpret_cast<const uint32_t*>(base + 0xB44458);
        }
    }
    if (flags & 0x20) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);
        ecx = val + ecx * 4;
    }

    int64_t rax_idx = static_cast<int64_t>(static_cast<int32_t>(ecx));
    r10d &= ~(1 << 0x1A); // btr r10d, 1Ah
    int64_t rdx = rax_idx * 0x88;
    const void* atomArray = *reinterpret_cast<const void* const*>(base + 0xC8E510 + r9);

    const void* rax_val = nullptr;
    if (atomArray) {
        rax_val = *reinterpret_cast<const void* const*>(reinterpret_cast<uintptr_t>(atomArray) + rdx + 0x18);
    }

    int64_t r8 = static_cast<int64_t>(r10d);
    *reinterpret_cast<uint64_t*>(base + 0xB44570 + (r8 * 8)) = ~0ULL;
    *reinterpret_cast<const void**>(base + 0xCA0370 + (r8 * 8)) = rax_val;
    *reinterpret_cast<uint32_t*>(base + 0xCA02A8) |= (1u << (r10d & 0x1F));
}

// 5. RVA 0x274670: BindAtomExt3
static void __fastcall Hook_0x274670(uint32_t slot, uint32_t index) {
    if (!s_haloReachBase) return;
    uintptr_t base = s_haloReachBase;
    int64_t rax = static_cast<int32_t>(index);
    int32_t r10d = static_cast<int32_t>(slot);
    uintptr_t r9 = static_cast<uintptr_t>(rax * 0x70);
    uint32_t ecx = 0;

    uint32_t flags = *reinterpret_cast<const uint32_t*>(base + 0xC8E4C0 + r9);
    if (flags & 0x100) {
        ecx = *reinterpret_cast<const uint32_t*>(base + 0xC8E518 + r9);
    }
    if (flags & 0x08) {
        ecx += ecx;
        if (index == 0x0C) {
            ecx += *reinterpret_cast<const uint32_t*>(base + 0xB44458);
        }
    }
    if (flags & 0x20) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);
        ecx = val + ecx * 4;
    }

    int64_t rax_idx = static_cast<int64_t>(static_cast<int32_t>(ecx));
    r10d &= ~(1 << 0x1B); // btr r10d, 1Bh
    int64_t rdx = rax_idx * 0x88;
    const void* atomArray = *reinterpret_cast<const void* const*>(base + 0xC8E510 + r9);

    const void* rax_val = nullptr;
    if (atomArray) {
        rax_val = *reinterpret_cast<const void* const*>(reinterpret_cast<uintptr_t>(atomArray) + rdx + 0x18);
    }

    int64_t r8 = static_cast<int64_t>(r10d);
    *reinterpret_cast<uint64_t*>(base + 0xB445A0 + (r8 * 8)) = ~0ULL;
    *reinterpret_cast<const void**>(base + 0xCA04A0 + (r8 * 8)) = rax_val;
    *reinterpret_cast<uint32_t*>(base + 0xCA02A4) |= (1u << (r10d & 0x1F));
}

// 6. RVA 0x2746F8: BindAtomExt4
static void* __fastcall Hook_0x2746F8(void** outPtr, uint32_t index) {
    if (!s_haloReachBase || !outPtr) return outPtr;
    uintptr_t base = s_haloReachBase;
    int64_t rax = static_cast<int32_t>(index);
    uintptr_t r9 = static_cast<uintptr_t>(rax * 0x70);
    uintptr_t r11 = base + 0xC8E4B0;
    uint32_t r8d = 0;

    uint32_t flags = *reinterpret_cast<const uint32_t*>(r9 + r11 + 0x10);
    if (flags & 0x100) {
        r8d = *reinterpret_cast<const uint32_t*>(r9 + r11 + 0x68);
    }
    if (flags & 0x08) {
        r8d += r8d;
        if (index == 0x0C) {
            r8d += *reinterpret_cast<const uint32_t*>(base + 0xB44458);
        }
    }
    if (flags & 0x20) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);
        r8d = val + r8d * 4;
    }

    int64_t rcx = static_cast<int64_t>(static_cast<int32_t>(r8d)) * 0x88;
    const void* atomArray = *reinterpret_cast<const void* const*>(r9 + r11 + 0x60);
    const void* res = nullptr;
    if (atomArray) {
        res = *reinterpret_cast<const void* const*>(reinterpret_cast<uintptr_t>(atomArray) + rcx + 0x18);
    }

    *outPtr = const_cast<void*>(res);
    return outPtr;
}

// 7. RVA 0x274758: GetAtom2C
static uint32_t __fastcall Hook_0x274758(uint32_t index) {
    if (!s_haloReachBase) return 0;
    uintptr_t base = s_haloReachBase;
    int64_t rax = static_cast<int32_t>(index);
    uintptr_t r9 = base + 0xC8E4B0;
    uintptr_t r8 = static_cast<uintptr_t>(rax * 0x70);
    uint32_t edx = 0;

    uint32_t flags = *reinterpret_cast<const uint32_t*>(r8 + r9 + 0x10);
    if (flags & 0x100) {
        edx = *reinterpret_cast<const uint32_t*>(r8 + r9 + 0x68);
    }
    if (flags & 0x08) {
        edx += edx;
        if (index == 0x0C) {
            edx += *reinterpret_cast<const uint32_t*>(base + 0xB44458);
        }
    }
    if (flags & 0x20) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);
        edx = val + edx * 4;
    }

    int64_t rcx = static_cast<int64_t>(static_cast<int32_t>(edx)) * 0x88;
    const void* atomArray = *reinterpret_cast<const void* const*>(r8 + r9 + 0x60);
    if (!atomArray) return 0;
    return *reinterpret_cast<const uint32_t*>(reinterpret_cast<uintptr_t>(atomArray) + rcx + 0x2C);
}

// 8. RVA 0x2747AC: GetAtom30
static uint32_t __fastcall Hook_0x2747AC(uint32_t index) {
    if (!s_haloReachBase) return 0;
    uintptr_t base = s_haloReachBase;
    int64_t rax = static_cast<int32_t>(index);
    uintptr_t r9 = base + 0xC8E4B0;
    uintptr_t r8 = static_cast<uintptr_t>(rax * 0x70);
    uint32_t edx = 0;

    uint32_t flags = *reinterpret_cast<const uint32_t*>(r8 + r9 + 0x10);
    if (flags & 0x100) {
        edx = *reinterpret_cast<const uint32_t*>(r8 + r9 + 0x68);
    }
    if (flags & 0x08) {
        edx += edx;
        if (index == 0x0C) {
            edx += *reinterpret_cast<const uint32_t*>(base + 0xB44458);
        }
    }
    if (flags & 0x20) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);
        edx = val + edx * 4;
    }

    int64_t rcx = static_cast<int64_t>(static_cast<int32_t>(edx)) * 0x88;
    const void* atomArray = *reinterpret_cast<const void* const*>(r8 + r9 + 0x60);
    if (!atomArray) return 0;
    return *reinterpret_cast<const uint32_t*>(reinterpret_cast<uintptr_t>(atomArray) + rcx + 0x30);
}

// 9. RVA 0x274800: GetAtom44
static uint32_t __fastcall Hook_0x274800(uint32_t index) {
    if (!s_haloReachBase) return 0;
    uintptr_t base = s_haloReachBase;
    int64_t rax = static_cast<int32_t>(index);
    uintptr_t r9 = base + 0xC8E4B0;
    uintptr_t r8 = static_cast<uintptr_t>(rax * 0x70);
    uint32_t edx = 0;

    uint32_t flags = *reinterpret_cast<const uint32_t*>(r8 + r9 + 0x10);
    if (flags & 0x100) {
        edx = *reinterpret_cast<const uint32_t*>(r8 + r9 + 0x68);
    }
    if (flags & 0x08) {
        edx += edx;
        if (index == 0x0C) {
            edx += *reinterpret_cast<const uint32_t*>(base + 0xB44458);
        }
    }
    if (flags & 0x20) {
        uint32_t val = *reinterpret_cast<const uint32_t*>(base + 0x4E38A08);
        edx = val + edx * 4;
    }

    int64_t rcx = static_cast<int64_t>(static_cast<int32_t>(edx)) * 0x88;
    const void* atomArray = *reinterpret_cast<const void* const*>(r8 + r9 + 0x60);
    if (!atomArray) return 0;
    return *reinterpret_cast<const uint32_t*>(reinterpret_cast<uintptr_t>(atomArray) + rcx + 0x44);
}

bool RasterizerAtomHooks::Install(HMODULE hReachModule) {
    if (!hReachModule) return false;
    s_haloReachBase = reinterpret_cast<uintptr_t>(hReachModule);

    NOBLE_INFO("Installing Rasterizer Atom safeguard detours...");

    struct HookEntry {
        uintptr_t rva;
        void* hookFn;
        void** origFn;
        const char* name;
    };

    HookEntry hooks[] = {
        { 0x2743A4, reinterpret_cast<void*>(&Hook_0x2743A4), &s_orig_0x2743A4, "Rasterizer_UpdatePool" },
        { 0x274488, reinterpret_cast<void*>(&Hook_0x274488), &s_orig_0x274488, "Rasterizer_BindAtom" },
        { 0x274564, reinterpret_cast<void*>(&Hook_0x274564), &s_orig_0x274564, "Rasterizer_BindAtomExt1" },
        { 0x2745E8, reinterpret_cast<void*>(&Hook_0x2745E8), &s_orig_0x2745E8, "Rasterizer_BindAtomExt2" },
        { 0x274670, reinterpret_cast<void*>(&Hook_0x274670), &s_orig_0x274670, "Rasterizer_BindAtomExt3" },
        { 0x2746F8, reinterpret_cast<void*>(&Hook_0x2746F8), &s_orig_0x2746F8, "Rasterizer_BindAtomExt4" },
        { 0x274758, reinterpret_cast<void*>(&Hook_0x274758), &s_orig_0x274758, "Rasterizer_GetAtom2C" },
        { 0x2747AC, reinterpret_cast<void*>(&Hook_0x2747AC), &s_orig_0x2747AC, "Rasterizer_GetAtom30" },
        { 0x274800, reinterpret_cast<void*>(&Hook_0x274800), &s_orig_0x274800, "Rasterizer_GetAtom44" },
    };

    for (const auto& entry : hooks) {
        void* target = reinterpret_cast<void*>(s_haloReachBase + entry.rva);
        if (!HookManager::InstallHook(target, entry.hookFn, entry.origFn, entry.name)) {
            NOBLE_WARN("Failed to install detour for {}", entry.name);
        }
    }

    NOBLE_INFO("All 9 Rasterizer Atom safeguard detours successfully active.");
    return true;
}

void RasterizerAtomHooks::Uninstall() {
    // Managed via HookManager::RemoveAllHooks
}

} // namespace noble

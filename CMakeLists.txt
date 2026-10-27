cmake_minimum_required(VERSION 3.20)
project(HaloReachCommunityEdition VERSION 0.7.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enforce UTF-8 and standard conformance on MSVC
if(MSVC)
    add_compile_options(/W4 /utf-8 /permissive- /Zc:__cplusplus /EHsc)
    add_compile_definitions(UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# 1. spdlog (Fast C++ logging)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.0
    GIT_SHALLOW TRUE
)

# 2. nlohmann_json (Modern JSON parser)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

# 3. MinHook (Minimalistic x86/x64 API Hooking Library)
FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG v1.3.3
    GIT_SHALLOW TRUE
)

# 4. Dear ImGui (Immediate Mode GUI for DirectX 11)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(spdlog nlohmann_json)

# Setup MinHook as static library
FetchContent_GetProperties(minhook)
if(NOT minhook_POPULATED)
    FetchContent_Populate(minhook)
    add_library(minhook STATIC
        ${minhook_SOURCE_DIR}/src/buffer.c
        ${minhook_SOURCE_DIR}/src/hook.c
        ${minhook_SOURCE_DIR}/src/trampoline.c
        ${minhook_SOURCE_DIR}/src/hde/hde64.c
        ${minhook_SOURCE_DIR}/src/hde/hde32.c
    )
    target_include_directories(minhook PUBLIC
        ${minhook_SOURCE_DIR}/include
        ${minhook_SOURCE_DIR}/src
    )
endif()

# Setup Dear ImGui with Win32 and DX11 backends
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    )
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC d3d11)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/host/SteamLocator.cpp
    src/host/OwnershipVerifier.cpp
    src/host/Window.cpp
    src/host/D3D11Renderer.cpp
    src/host/Loader.cpp
    src/hooks/SigScanner.cpp
    src/hooks/HookManager.cpp
    src/hooks/LobbyPatches.cpp
    src/hooks/ForgePatches.cpp
    src/vfs/VFS.cpp
    src/engine/BlamEngine.cpp
    src/engine/ReachHost.cpp
    src/ui/UIManager.cpp
    src/ui/ServerBrowser.cpp
    src/ui/ArmorCustomizer.cpp
    src/ui/WorkshopManager.cpp
    src/ui/MissionLauncher.cpp
    src/network/UPnP.cpp
    src/network/SessionManager.cpp
    src/audio/VgmAudioHook.cpp
    src/ui/LoadScreen.cpp
)

set(HEADERS
    src/common/Logger.hpp
    src/common/Types.hpp
    src/common/Version.hpp
    src/host/SteamLocator.hpp
    src/host/OwnershipVerifier.hpp
    src/host/Window.hpp
    src/host/D3D11Renderer.hpp
    src/host/Loader.hpp
    src/hooks/SigScanner.hpp
    src/hooks/HookManager.hpp
    src/hooks/LobbyPatches.hpp
    src/hooks/ForgePatches.hpp
    src/vfs/VFS.hpp
    src/engine/BlamEngine.hpp
    src/engine/ReachHost.hpp
    src/ui/UIManager.hpp
    src/ui/ServerBrowser.hpp
    src/ui/ArmorCustomizer.hpp
    src/ui/WorkshopManager.hpp
    src/ui/MissionLauncher.hpp
    src/ui/LoadScreen.hpp
    src/network/UPnP.hpp
    src/network/SessionManager.hpp
    src/audio/VgmAudioHook.hpp
)

add_executable(noble-client WIN32 ${SOURCES} ${HEADERS})

target_include_directories(noble-client PRIVATE
    src
)

target_link_libraries(noble-client PRIVATE
    d3d11
    dxgi
    d3dcompiler
    ws2_32
    iphlpapi
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    minhook
    imgui
    dbghelp
)

target_link_options(noble-client PRIVATE /MAP)

# -----------------------------------------------------------------------------
# Headless Dedicated Server Binary (noble-dedicated.exe)
# -----------------------------------------------------------------------------
set(DEDICATED_SOURCES
    src/dedicated/main.cpp
    src/dedicated/DedicatedServer.cpp
    src/host/SteamLocator.cpp
    src/host/OwnershipVerifier.cpp
    src/host/Loader.cpp
    src/hooks/SigScanner.cpp
    src/hooks/HookManager.cpp
    src/hooks/LobbyPatches.cpp
    src/hooks/ForgePatches.cpp
    src/vfs/VFS.cpp
    src/network/UPnP.cpp
    src/network/SessionManager.cpp
)

set(DEDICATED_HEADERS
    src/dedicated/DedicatedServer.hpp
    src/common/Logger.hpp
    src/common/Types.hpp
    src/common/Version.hpp
    src/host/SteamLocator.hpp
    src/host/OwnershipVerifier.hpp
    src/host/Loader.hpp
    src/hooks/SigScanner.hpp
    src/hooks/HookManager.hpp
    src/hooks/LobbyPatches.hpp
    src/hooks/ForgePatches.hpp
    src/vfs/VFS.hpp
    src/network/UPnP.hpp
    src/network/SessionManager.hpp
)

add_executable(noble-dedicated ${DEDICATED_SOURCES} ${DEDICATED_HEADERS})

target_include_directories(noble-dedicated PRIVATE
    src
)

target_link_libraries(noble-dedicated PRIVATE
    ws2_32
    iphlpapi
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    minhook
    dbghelp
)

cmake_minimum_required(VERSION 3.20)
project(HaloReachCommunityEdition VERSION 0.2.0 LANGUAGES C CXX)

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

FetchContent_MakeAvailable(spdlog nlohmann_json)

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
    src/vfs/VFS.cpp
    src/engine/BlamEngine.cpp
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
    src/vfs/VFS.hpp
    src/engine/BlamEngine.hpp
)

add_executable(noble-client WIN32 ${SOURCES} ${HEADERS})

target_include_directories(noble-client PRIVATE
    src
)

target_link_libraries(noble-client PRIVATE
    d3d11
    dxgi
    d3dcompiler
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    minhook
)

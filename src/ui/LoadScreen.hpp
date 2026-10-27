#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>
#include <chrono>

namespace noble {

using Microsoft::WRL::ComPtr;

struct LoadScreenVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

class LoadScreen {
public:
    static LoadScreen& Get();

    LoadScreen();
    ~LoadScreen() = default;

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Shutdown();

    // Map request event triggered when loading starts (or from Slot 48 / LaunchMap)
    bool LoadDots(const std::filesystem::path& dotsFilePath);
    bool RequestMapDots(const std::string& mapName, const std::filesystem::path& dataDir);

    void SetLoadProgress(float fraction);
    float GetLoadProgress() const { return m_progress; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    // Renders the rotating 3D dot cloud during loading screen phase
    void Render(float width, float height);

private:
    bool CreateShadersAndBuffers();

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_constantBuffer;
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11DepthStencilState> m_depthState;

    std::vector<LoadScreenVertex> m_vertices;
    uint32_t m_vertexCount = 0;
    float m_center[3] = { 0.0f, 0.0f, 0.0f };
    float m_radius = 1.0f;

    bool m_isInitialized = false;
    bool m_active = false;
    float m_progress = 0.0f;
    std::chrono::steady_clock::time_point m_startTime;
};

} // namespace noble

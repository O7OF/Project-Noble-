#include "LoadScreen.hpp"
#include "common/Logger.hpp"
#include <d3dcompiler.h>
#include <fstream>
#include <cmath>
#include <algorithm>

namespace noble {

// Constant buffer matching host-probe Gpu::draw shader inputs
struct alignas(16) LoadScreenCB {
    float viewProj[16];   // 4x4 matrix
    float cameraPos[4];
    float params[4];      // x: time, y: progress, z: dotSize, w: unused
    float color[4];       // Reach HUD cyan / blue color
};

// HLSL Shaders embedded as strings for runtime compilation via D3DCompile
static const char* s_loadScreenShaderCode = R"(
cbuffer SceneConstants : register(b0)
{
    float4x4 g_viewProj;
    float4 g_cameraPos;
    float4 g_params;    // x: time, y: progress, z: dotSize, w: unused
    float4 g_color;
};

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float size : PSIZE;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = float4(input.pos, 1.0f);
    output.pos = mul(worldPos, g_viewProj);
    
    // Depth attenuation and subtle pulse
    float dist = length(input.pos - g_cameraPos.xyz);
    float alpha = saturate(1.2f - (dist * 0.005f));
    
    output.color = g_color * float4(1.0f, 1.0f, 1.0f, alpha);
    output.size = g_params.z;
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.color;
}
)";

LoadScreen& LoadScreen::Get() {
    static LoadScreen s_instance;
    return s_instance;
}

LoadScreen::LoadScreen() : m_startTime(std::chrono::steady_clock::now()) {}

bool LoadScreen::Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (m_isInitialized) return true;
    m_device = device;
    m_context = context;

    if (!CreateShadersAndBuffers()) {
        NOBLE_ERROR("LoadScreen: Failed to compile shaders or create D3D11 resources.");
        return false;
    }

    m_isInitialized = true;
    NOBLE_INFO("LoadScreen: Initialized authentic Halo Reach 3D point cloud loading screen renderer.");
    return true;
}

void LoadScreen::Shutdown() {
    m_vertexBuffer.Reset();
    m_constantBuffer.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_inputLayout.Reset();
    m_blendState.Reset();
    m_depthState.Reset();
    m_vertices.clear();
    m_vertexCount = 0;
    m_isInitialized = false;
    m_active = false;
}

bool LoadScreen::CreateShadersAndBuffers() {
    if (!m_device) return false;

    // Compile Vertex Shader
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompile(
        s_loadScreenShaderCode,
        strlen(s_loadScreenShaderCode),
        "LoadScreenShaders",
        nullptr,
        nullptr,
        "VSMain",
        "vs_4_0",
        0,
        0,
        vsBlob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            NOBLE_ERROR("LoadScreen: Vertex Shader error: {}", (char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(hr)) return false;

    // Compile Pixel Shader
    ComPtr<ID3DBlob> psBlob;
    hr = D3DCompile(
        s_loadScreenShaderCode,
        strlen(s_loadScreenShaderCode),
        "LoadScreenShaders",
        nullptr,
        nullptr,
        "PSMain",
        "ps_4_0",
        0,
        0,
        psBlob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            NOBLE_ERROR("LoadScreen: Pixel Shader error: {}", (char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
    if (FAILED(hr)) return false;

    // Input layout for LoadScreenVertex (32 bytes: pos, norm, uv)
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = m_device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_inputLayout.GetAddressOf());
    if (FAILED(hr)) return false;

    // Constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(LoadScreenCB);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = m_device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    // Blend state: Alpha blending
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = m_device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());
    if (FAILED(hr)) return false;

    // Depth stencil state: disable depth test for additive dot render
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    hr = m_device->CreateDepthStencilState(&depthDesc, m_depthState.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
}

bool LoadScreen::LoadDots(const std::filesystem::path& dotsFilePath) {
    if (!std::filesystem::exists(dotsFilePath)) {
        NOBLE_WARN("LoadScreen: Dots file does not exist: {}", dotsFilePath.string());
        return false;
    }

    std::ifstream file(dotsFilePath, std::ios::binary);
    if (!file) {
        NOBLE_ERROR("LoadScreen: Failed to open dots file: {}", dotsFilePath.string());
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    if (fileSize < 12) {
        NOBLE_WARN("LoadScreen: Dots file size too small: {}", fileSize);
        return false;
    }

    // Header check: 0x3153544f445252 ("RRDDOTS1")
    uint64_t magic = 0;
    file.read(reinterpret_cast<char*>(&magic), sizeof(uint64_t));
    if (magic != 0x3153544F445252ULL) {
        NOBLE_WARN("LoadScreen: Invalid header magic 0x{:X} (expected RRDDOTS1)", magic);
        return false;
    }

    uint32_t pointCount = 0;
    file.read(reinterpret_cast<char*>(&pointCount), sizeof(uint32_t));

    size_t maxPoints = (fileSize - 12) / 12;
    if (pointCount > maxPoints) pointCount = static_cast<uint32_t>(maxPoints);

    if (pointCount == 0) return false;

    std::vector<float> coords(pointCount * 3);
    file.read(reinterpret_cast<char*>(coords.data()), pointCount * 12);

    // Compute bounding box and center
    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;
    float minZ = 1e9f, maxZ = -1e9f;

    for (size_t i = 0; i < pointCount; ++i) {
        float x = coords[i * 3 + 0];
        float y = coords[i * 3 + 1];
        float z = coords[i * 3 + 2];
        if (x < minX) minX = x; if (x > maxX) maxX = x;
        if (y < minY) minY = y; if (y > maxY) maxY = y;
        if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
    }

    m_center[0] = (minX + maxX) * 0.5f;
    m_center[1] = (minY + maxY) * 0.5f;
    m_center[2] = (minZ + maxZ) * 0.5f;

    float dx = maxX - minX;
    float dy = maxY - minY;
    float dz = maxZ - minZ;
    m_radius = std::sqrt(dx * dx + dy * dy + dz * dz) * 0.45f;
    if (m_radius < 1.0f) m_radius = 1.0f;

    // Build vertex list
    m_vertices.resize(pointCount);
    for (size_t i = 0; i < pointCount; ++i) {
        m_vertices[i].x = coords[i * 3 + 0];
        m_vertices[i].y = coords[i * 3 + 1];
        m_vertices[i].z = coords[i * 3 + 2];
        m_vertices[i].nx = 0.0f;
        m_vertices[i].ny = 1.0f;
        m_vertices[i].nz = 0.0f;
        m_vertices[i].u = 0.0f;
        m_vertices[i].v = 0.0f;
    }
    m_vertexCount = pointCount;

    // Upload to D3D11 vertex buffer
    if (m_device) {
        m_vertexBuffer.Reset();
        D3D11_BUFFER_DESC vbDesc = {};
        vbDesc.ByteWidth = static_cast<UINT>(m_vertices.size() * sizeof(LoadScreenVertex));
        vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = m_vertices.data();

        HRESULT hr = m_device->CreateBuffer(&vbDesc, &initData, m_vertexBuffer.GetAddressOf());
        if (FAILED(hr)) {
            NOBLE_ERROR("LoadScreen: Failed to create vertex buffer for {} points", pointCount);
            return false;
        }
    }

    m_active = true;
    m_progress = 0.04f;
    m_startTime = std::chrono::steady_clock::now();

    NOBLE_INFO("LoadScreen: Loaded {} wireframe dots from '{}' (Center: [{:.1f}, {:.1f}, {:.1f}], Radius: {:.1f})",
        pointCount, dotsFilePath.filename().string(), m_center[0], m_center[1], m_center[2], m_radius);
    return true;
}

bool LoadScreen::RequestMapDots(const std::string& mapName, const std::filesystem::path& dataDir) {
    std::string cleanName = mapName;
    size_t lastSlash = cleanName.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        cleanName = cleanName.substr(lastSlash + 1);
    }
    size_t dotMap = cleanName.find(".map");
    if (dotMap != std::string::npos) {
        cleanName = cleanName.substr(0, dotMap);
    }

    std::filesystem::path dotsPath = dataDir / "loadscreen" / (cleanName + ".dots");
    if (std::filesystem::exists(dotsPath)) {
        return LoadDots(dotsPath);
    }

    // Default fallback to sword slayer or forge if specific map .dots doesn't exist
    std::filesystem::path defaultDots = dataDir / "loadscreen" / "forge_halo.dots";
    if (std::filesystem::exists(defaultDots)) {
        return LoadDots(defaultDots);
    }

    return false;
}

void LoadScreen::SetLoadProgress(float fraction) {
    m_progress = fraction;
    if (m_progress >= 1.0f) {
        m_active = false;
    }
}

void LoadScreen::Render(float width, float height) {
    if (!m_active || !m_isInitialized || !m_context || !m_vertexBuffer || m_vertexCount == 0) return;

    auto now = std::chrono::steady_clock::now();
    float timeSec = std::chrono::duration<float>(now - m_startTime).count();

    // Camera rotation matching host-probe Gpu::draw: sincosf(time * 0.06 + 0.6)
    float angle = timeSec * 0.45f + 0.6f;
    float dist = m_radius * 2.1f;
    float camX = m_center[0] + dist * std::cos(angle) * 0.8775f;
    float camY = m_center[1] + dist * 0.4794f;
    float camZ = m_center[2] + dist * std::sin(angle) * 0.8775f;

    // View matrix: LookAt(cam, center, up)
    float fwd[3] = { m_center[0] - camX, m_center[1] - camY, m_center[2] - camZ };
    float fwdLen = std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1] + fwd[2] * fwd[2]);
    if (fwdLen < 1e-5f) fwdLen = 1e-5f;
    fwd[0] /= fwdLen; fwd[1] /= fwdLen; fwd[2] /= fwdLen;

    float up[3] = { 0.0f, 1.0f, 0.0f };
    float right[3] = { up[1] * fwd[2] - up[2] * fwd[1], up[2] * fwd[0] - up[0] * fwd[2], up[0] * fwd[1] - up[1] * fwd[0] };
    float rLen = std::sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
    if (rLen < 1e-5f) rLen = 1e-5f;
    right[0] /= rLen; right[1] /= rLen; right[2] /= rLen;

    float trueUp[3] = { fwd[1] * right[2] - fwd[2] * right[1], fwd[2] * right[0] - fwd[0] * right[2], fwd[0] * right[1] - fwd[1] * right[0] };

    // Projection matrix: Perspective 60 deg
    float aspect = (height > 0.0f) ? (width / height) : (16.0f / 9.0f);
    float fov = 1.04719755f; // 60 degrees in radians
    float tanHalfFov = std::tan(fov * 0.5f);
    float nearZ = 0.5f;
    float farZ = 10000.0f;

    float proj[16] = { 0 };
    proj[0] = 1.0f / (aspect * tanHalfFov);
    proj[5] = 1.0f / tanHalfFov;
    proj[10] = farZ / (farZ - nearZ);
    proj[11] = 1.0f;
    proj[14] = (-nearZ * farZ) / (farZ - nearZ);

    float view[16] = { 0 };
    view[0] = right[0];  view[4] = right[1];  view[8]  = right[2];  view[12] = -(right[0] * camX + right[1] * camY + right[2] * camZ);
    view[1] = trueUp[0]; view[5] = trueUp[1]; view[9]  = trueUp[2]; view[13] = -(trueUp[0] * camX + trueUp[1] * camY + trueUp[2] * camZ);
    view[2] = fwd[0];    view[6] = fwd[1];    view[10] = fwd[2];    view[14] = -(fwd[0] * camX + fwd[1] * camY + fwd[2] * camZ);
    view[3] = 0.0f;      view[7] = 0.0f;      view[11] = 0.0f;      view[15] = 1.0f;

    // ViewProj = View * Proj
    LoadScreenCB cbData = {};
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            cbData.viewProj[r * 4 + c] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                cbData.viewProj[r * 4 + c] += view[r * 4 + k] * proj[k * 4 + c];
            }
        }
    }

    cbData.cameraPos[0] = camX;
    cbData.cameraPos[1] = camY;
    cbData.cameraPos[2] = camZ;
    cbData.cameraPos[3] = 1.0f;

    cbData.params[0] = timeSec;
    cbData.params[1] = m_progress;
    cbData.params[2] = 2.0f; // Dot point size
    cbData.params[3] = 0.0f;

    // Authentic Reach cyan wireframe color
    cbData.color[0] = 0.15f;
    cbData.color[1] = 0.75f;
    cbData.color[2] = 0.95f;
    cbData.color[3] = 0.85f;

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, &cbData, sizeof(LoadScreenCB));
        m_context->Unmap(m_constantBuffer.Get(), 0);
    }

    // Bind pipeline state
    UINT stride = sizeof(LoadScreenVertex);
    UINT offset = 0;
    ID3D11Buffer* vbs[] = { m_vertexBuffer.Get() };
    m_context->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    ID3D11Buffer* cbs[] = { m_constantBuffer.Get() };
    m_context->VSSetConstantBuffers(0, 1, cbs);

    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->PSSetConstantBuffers(0, 1, cbs);

    float blendFactor[4] = { 0, 0, 0, 0 };
    m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);
    m_context->OMSetDepthStencilState(m_depthState.Get(), 0);

    // Draw rotating point cloud particles
    m_context->Draw(m_vertexCount, 0);
}

} // namespace noble

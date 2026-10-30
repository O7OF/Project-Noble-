#include "ObjModelPreview.hpp"

#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <objbase.h>
#include <wincodec.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <limits>
#include <wrl/client.h>

namespace noble {
namespace {

using Microsoft::WRL::ComPtr;
using namespace DirectX;

constexpr UINT kPreviewWidth = 960;
constexpr UINT kPreviewHeight = 1080;

struct alignas(16) PreviewConstants {
	XMFLOAT4X4 worldViewProjection;
	XMFLOAT4X4 world;
	XMFLOAT4 diffuse;
	XMFLOAT4 primaryColor;
	XMFLOAT4 secondaryColor;
	XMFLOAT4 detailColor;
	XMFLOAT4 visorColor;
	XMFLOAT4 options; // x = hasDiffuse, y = hasColorControl, z = hasNormalMap, w = materialMode
};

static_assert(sizeof(PreviewConstants) % 16 == 0);

const char* kShaderSource = R"(
cbuffer PreviewConstants : register(b0)
{
	float4x4 g_worldViewProjection;
	float4x4 g_world;
	float4 g_diffuse;
	float4 g_primaryColor;
	float4 g_secondaryColor;
	float4 g_detailColor;
	float4 g_visorColor;
	float4 g_options;
};

Texture2D g_diffuseTexture : register(t0);
Texture2D g_colorControlTexture : register(t1);
Texture2D g_normalTexture : register(t2);
SamplerState g_sampler : register(s0);

struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
	float3 worldPos : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
	VSOutput output;
	float4 worldPos = mul(float4(input.position, 1.0f), g_world);
	output.position = mul(float4(input.position, 1.0f), g_worldViewProjection);
	output.normal = normalize(mul(float4(input.normal, 0.0f), g_world).xyz);
	output.texcoord = input.texcoord;
	output.worldPos = worldPos.xyz;
	return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
	float3 N = normalize(input.normal);
	float3 V = normalize(float3(0.0f, 0.08f, -3.2f) - input.worldPos);
	if (dot(N, V) < 0.0f)
		N = -N;

	if (g_options.z > 0.5f)
	{
		float3 mapN = g_normalTexture.Sample(g_sampler, input.texcoord).xyz * 2.0f - 1.0f;
		mapN.y = -mapN.y;
		mapN.xy *= 1.15f;
		mapN = normalize(mapN);

		float3 dp1 = ddx(input.worldPos);
		float3 dp2 = ddy(input.worldPos);
		float2 duv1 = ddx(input.texcoord);
		float2 duv2 = ddy(input.texcoord);
		float3 dp2perp = cross(dp2, N);
		float3 dp1perp = cross(N, dp1);
		float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
		float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
		float invmax = rsqrt(max(dot(T, T), dot(B, B)) + 1e-8f);
		float3x3 TBN = float3x3(T * invmax, B * invmax, N);
		N = normalize(mul(mapN, TBN));
	}

	// 3-point MCC Armory studio lighting:
	// Key light (cool upper-left-front), Fill light (soft front-right), Rim light (Halo ring cyan-blue back-left)
	float3 keyDir = normalize(float3(-0.48f, 0.62f, -0.62f));
	float3 fillDir = normalize(float3(0.65f, 0.15f, -0.74f));
	float3 rimDir = normalize(float3(-0.72f, 0.35f, 0.60f));

	float NdotKey = saturate(dot(N, keyDir));
	float NdotFill = saturate(dot(N, fillDir));
	float NdotRim = saturate(dot(N, rimDir));
	float fresnel = pow(1.0f - saturate(dot(N, V)), 3.0f);

	float3 HKey = normalize(keyDir + V);
	float3 HFill = normalize(fillDir + V);
	float specKey = pow(saturate(dot(N, HKey)), 24.0f);
	float specWide = pow(saturate(dot(N, HKey)), 6.0f);

	int mode = (int)round(g_options.w);

	// Mode 5: Emissive armor lights
	if (mode == 5)
	{
		return float4(0.42f, 0.78f, 1.0f, 1.0f);
	}

	// Mode 1: Reflective metallic Visor
	if (mode == 1)
	{
		float3 visorBase = g_visorColor.rgb;
		float3 envDark = visorBase * 0.28f;
		float3 envBright = lerp(visorBase * 1.35f, float3(1.0f, 0.98f, 0.90f), 0.28f);
		float2 reflUv = N.xy * 0.5f + 0.5f;
		float horizon = smoothstep(-0.15f, 0.45f, N.y);
		float3 visorCol = lerp(envDark, visorBase, 0.45f + 0.55f * NdotKey);
		visorCol = lerp(visorCol, envBright, fresnel * 0.65f + horizon * 0.22f);
		float visorSpec = pow(saturate(dot(N, HKey)), 64.0f) * 1.35f + pow(saturate(dot(N, HFill)), 28.0f) * 0.35f;
		visorCol += visorSpec * lerp(visorBase, float3(1.0f, 1.0f, 1.0f), 0.55f);
		visorCol += float3(0.30f, 0.55f, 0.85f) * (NdotRim * fresnel * 0.55f);
		return float4(saturate(visorCol), 1.0f);
	}

	float4 texSample = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (g_options.x > 0.5f)
		texSample = g_diffuseTexture.Sample(g_sampler, input.texcoord);

	// Contrast-enhance AO/wear in Reach diffuse maps
	float3 aoWear = pow(saturate(texSample.rgb), 1.35f);
	float3 baseColor = aoWear * g_diffuse.rgb;
	float specStrength = 0.28f;

	if (mode == 2)
	{
		// Rubber undersuit / techsuit / helmet interior
		baseColor = aoWear * float3(0.11f, 0.12f, 0.14f);
		specStrength = 0.14f;
	}
	else if (mode == 3)
	{
		// Tactical gear / pouches / unpainted titanium / prosthetic arm / attachments
		baseColor = aoWear * float3(0.25f, 0.27f, 0.30f);
		specStrength = 0.32f;
	}
	else if (mode == 6)
	{
		// Haunted skull bone ivory
		baseColor = aoWear * float3(0.72f, 0.67f, 0.56f);
		specStrength = 0.18f;
	}
	else if (g_options.y > 0.5f)
	{
		// Painted armor with _cc mask:
		// R = Primary, G = Secondary, B = Detail, Black (unpainted) = Dark Gunmetal (g_detailColor)
		float3 mask = g_colorControlTexture.Sample(g_sampler, input.texcoord).rgb;
		float coverage = saturate(mask.r + mask.g + mask.b);
		float3 paintedColor = (g_primaryColor.rgb * mask.r + g_secondaryColor.rgb * mask.g +
			g_detailColor.rgb * mask.b) / max(mask.r + mask.g + mask.b, 0.0001f);
		float3 armorTint = lerp(g_detailColor.rgb, paintedColor, coverage);
		baseColor = aoWear * armorTint;
		specStrength = lerp(0.36f, 0.26f, coverage);
	}
	else if (mode == 4)
	{
		// Painted armor piece without a separate _cc texture
		baseColor = aoWear * g_primaryColor.rgb;
		specStrength = 0.28f;
	}
	else if (mode == 7)
	{
		// Secondary-painted armor piece without a separate _cc texture
		baseColor = aoWear * g_secondaryColor.rgb;
		specStrength = 0.28f;
	}

	float3 ambient = float3(0.16f, 0.18f, 0.22f) + 0.06f * saturate(N.y * 0.5f + 0.5f);
	float3 keyLight = float3(0.96f, 0.98f, 1.00f) * (NdotKey * 0.78f);
	float3 fillLight = float3(0.32f, 0.38f, 0.48f) * (NdotFill * 0.34f);
	float3 rimLight = float3(0.38f, 0.62f, 0.92f) * (NdotRim * (0.25f + 0.45f * fresnel));

	float3 lit = baseColor * (ambient + keyLight + fillLight) + rimLight * 0.28f;
	float3 specCol = lerp(baseColor, float3(0.90f, 0.95f, 1.0f), 0.65f);
	lit += specCol * (specKey * specStrength + specWide * specStrength * 0.25f);

	return float4(saturate(lit), 1.0f);
}
)";

bool ContainsIgnoreCase(std::string_view haystack, std::string_view needle) {
	if (needle.empty() || haystack.size() < needle.size()) return false;
	auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
		[](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
	return it != haystack.end();
}

bool EndsWithIgnoreCase(std::string_view str, std::string_view suffix) {
	if (str.size() < suffix.size()) return false;
	return ContainsIgnoreCase(str.substr(str.size() - suffix.size()), suffix);
}

bool ShouldSkipDecalCardMaterial(std::string_view materialName) {
	return ContainsIgnoreCase(materialName, "illum_cards") ||
		   ContainsIgnoreCase(materialName, "emblem_patch") ||
		   ContainsIgnoreCase(materialName, "carter_259") ||
		   ContainsIgnoreCase(materialName, "unsc_decals") ||
		   ContainsIgnoreCase(materialName, "spartan_decals") ||
		   ContainsIgnoreCase(materialName, "bullet_burn_decal") ||
		   ContainsIgnoreCase(materialName, "scratch_decal") ||
		   ContainsIgnoreCase(materialName, "paintwear") ||
		   ContainsIgnoreCase(materialName, "paint_emile") ||
		   ContainsIgnoreCase(materialName, "paint_jun") ||
		   ContainsIgnoreCase(materialName, "paint_carter");
}

int ClassifyMaterialMode(std::string_view materialName, bool hasColorControl, bool useSecondaryColor) {
	if (ContainsIgnoreCase(materialName, "lights") || ContainsIgnoreCase(materialName, "shield_display")) {
		return 5; // Emissive lights
	}
	if (ContainsIgnoreCase(materialName, "visor") && !ContainsIgnoreCase(materialName, "interior")) {
		return 1; // Reflective Visor
	}
	if (ContainsIgnoreCase(materialName, "rubber") ||
		ContainsIgnoreCase(materialName, "bodysuit") ||
		ContainsIgnoreCase(materialName, "interior") ||
		ContainsIgnoreCase(materialName, "black")) {
		return 2; // Dark matte rubber undersuit
	}
	if (ContainsIgnoreCase(materialName, "skull_main") ||
		ContainsIgnoreCase(materialName, "skull_teeth") ||
		ContainsIgnoreCase(materialName, "skull_port") ||
		ContainsIgnoreCase(materialName, "skull_emile")) {
		return 6; // Bone / skull
	}
	if (EndsWithIgnoreCase(materialName, "_metal") ||
		ContainsIgnoreCase(materialName, "untinted") ||
		ContainsIgnoreCase(materialName, "robotarm") ||
		ContainsIgnoreCase(materialName, "robo_arm")) {
		return 3; // Unpainted gunmetal / prosthetic arm
	}
	if (hasColorControl) {
		return 0; // Painted armor with _cc mask
	}
	if (ContainsIgnoreCase(materialName, "knife") ||
		ContainsIgnoreCase(materialName, "kukri") ||
		ContainsIgnoreCase(materialName, "pouch") ||
		ContainsIgnoreCase(materialName, "grenade") ||
		ContainsIgnoreCase(materialName, "ammopack") ||
		ContainsIgnoreCase(materialName, "backpack") ||
		ContainsIgnoreCase(materialName, "harness") ||
		ContainsIgnoreCase(materialName, "shotgun") ||
		ContainsIgnoreCase(materialName, "flask") ||
		ContainsIgnoreCase(materialName, "packs") ||
		ContainsIgnoreCase(materialName, "ear_piece") ||
		ContainsIgnoreCase(materialName, "ear_peice") ||
		ContainsIgnoreCase(materialName, "_aug") ||
		ContainsIgnoreCase(materialName, "aug_") ||
		ContainsIgnoreCase(materialName, "flare") ||
		ContainsIgnoreCase(materialName, "chesttech") ||
		ContainsIgnoreCase(materialName, "helmettech") ||
		ContainsIgnoreCase(materialName, "gauntlets") ||
		ContainsIgnoreCase(materialName, "sniper_li")) {
		return 3; // Tactical gear / attachments
	}
	return useSecondaryColor ? 7 : 4;
}

bool CompileShader(const char* entryPoint, const char* profile, ComPtr<ID3DBlob>& bytecode) {
	ComPtr<ID3DBlob> errors;
	const HRESULT hr = D3DCompile(kShaderSource, std::strlen(kShaderSource), "ObjModelPreview", nullptr, nullptr,
								  entryPoint, profile, D3DCOMPILE_ENABLE_STRICTNESS, 0,
								  bytecode.GetAddressOf(), errors.GetAddressOf());
	return SUCCEEDED(hr);
}

} // namespace

bool ObjModelPreview::Initialize(ID3D11Device* device) {
	if (m_isInitialized) return true;
	if (!device) return false;
	m_device = device;
	m_rtWidth = kPreviewWidth;
	m_rtHeight = kPreviewHeight;
	if (!CreatePipeline() || !CreatePreviewTarget()) {
		m_device = nullptr;
		return false;
	}
	m_isInitialized = true;
	return true;
}

bool ObjModelPreview::CreatePipeline() {
	ComPtr<ID3DBlob> vertexBytecode;
	ComPtr<ID3DBlob> pixelBytecode;
	if (!CompileShader("VSMain", "vs_4_0", vertexBytecode) ||
		!CompileShader("PSMain", "ps_4_0", pixelBytecode)) return false;

	if (FAILED(m_device->CreateVertexShader(vertexBytecode->GetBufferPointer(), vertexBytecode->GetBufferSize(),
											nullptr, m_vertexShader.GetAddressOf())) ||
		FAILED(m_device->CreatePixelShader(pixelBytecode->GetBufferPointer(), pixelBytecode->GetBufferSize(),
										   nullptr, m_pixelShader.GetAddressOf()))) return false;

	const D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ObjVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ObjVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ObjVertex, texcoord), D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	if (FAILED(m_device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexBytecode->GetBufferPointer(),
										   vertexBytecode->GetBufferSize(), m_inputLayout.GetAddressOf()))) return false;

	D3D11_BUFFER_DESC constantDesc{};
	constantDesc.ByteWidth = sizeof(PreviewConstants);
	constantDesc.Usage = D3D11_USAGE_DEFAULT;
	constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (FAILED(m_device->CreateBuffer(&constantDesc, nullptr, m_constantBuffer.GetAddressOf()))) return false;

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(m_device->CreateSamplerState(&samplerDesc, m_sampler.GetAddressOf()))) return false;

	D3D11_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	if (FAILED(m_device->CreateDepthStencilState(&depthDesc, m_depthState.GetAddressOf()))) return false;

	D3D11_RASTERIZER_DESC rasterDesc{};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthClipEnable = TRUE;
	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_rasterizerState.GetAddressOf()))) return false;
	return true;
}

bool ObjModelPreview::CreatePreviewTarget() {
	D3D11_TEXTURE2D_DESC colorDesc{};
	colorDesc.Width = m_rtWidth;
	colorDesc.Height = m_rtHeight;
	colorDesc.MipLevels = 1;
	colorDesc.ArraySize = 1;
	colorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorDesc.SampleDesc.Count = 1;
	colorDesc.Usage = D3D11_USAGE_DEFAULT;
	colorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (FAILED(m_device->CreateTexture2D(&colorDesc, nullptr, m_previewTexture.GetAddressOf())) ||
		FAILED(m_device->CreateRenderTargetView(m_previewTexture.Get(), nullptr, m_previewRtv.GetAddressOf())) ||
		FAILED(m_device->CreateShaderResourceView(m_previewTexture.Get(), nullptr, m_previewSrv.GetAddressOf()))) return false;

	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = m_rtWidth;
	depthDesc.Height = m_rtHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (FAILED(m_device->CreateTexture2D(&depthDesc, nullptr, m_depthTexture.GetAddressOf())) ||
		FAILED(m_device->CreateDepthStencilView(m_depthTexture.Get(), nullptr, m_depthDsv.GetAddressOf()))) return false;
	return true;
}

bool ObjModelPreview::CreateTexture(const std::filesystem::path& path, ID3D11ShaderResourceView** texture) {
	if (!texture) return false;
	*texture = nullptr;
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	const bool uninitializeCom = SUCCEEDED(comResult);
	if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) return false;

	bool success = false;
	do {
		ComPtr<IWICImagingFactory> factory;
		if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
									IID_PPV_ARGS(factory.GetAddressOf())))) break;
		ComPtr<IWICBitmapDecoder> decoder;
		if (FAILED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
													  WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf()))) break;
		ComPtr<IWICBitmapFrameDecode> frame;
		if (FAILED(decoder->GetFrame(0, frame.GetAddressOf()))) break;
		UINT width = 0, height = 0;
		if (FAILED(frame->GetSize(&width, &height)) || width == 0 || height == 0 || width > 4096 || height > 4096) break;
		ComPtr<IWICFormatConverter> converter;
		if (FAILED(factory->CreateFormatConverter(converter.GetAddressOf())) ||
			FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
										nullptr, 0.0, WICBitmapPaletteTypeCustom))) break;
		const uint64_t byteCount = static_cast<uint64_t>(width) * height * 4;
		if (byteCount > std::numeric_limits<UINT>::max()) break;
		std::vector<uint8_t> pixels(static_cast<size_t>(byteCount));
		if (FAILED(converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(byteCount), pixels.data()))) break;

		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA init{};
		init.pSysMem = pixels.data();
		init.SysMemPitch = width * 4;
		ComPtr<ID3D11Texture2D> image;
		if (FAILED(m_device->CreateTexture2D(&desc, &init, image.GetAddressOf())) ||
			FAILED(m_device->CreateShaderResourceView(image.Get(), nullptr, texture))) break;
		success = true;
	} while (false);

	if (uninitializeCom) CoUninitialize();
	return success;
}

bool ObjModelPreview::CreateGpuModel(const ObjModel& model, std::string& error) {
	std::vector<GpuMesh> meshes;
	std::unordered_map<std::string, GpuMaterial> materials;
	for (const auto& source : model.materials) {
		GpuMaterial material;
		std::copy(std::begin(source.diffuse), std::end(source.diffuse), material.diffuse);
		material.diffuseTexturePath = source.diffuseTexture;
		material.colorControlTexturePath = source.colorControlTexture;
		material.normalTexturePath = source.normalTexture;
		materials.emplace(source.name, std::move(material));
	}

	for (const auto& source : model.meshes) {
		if (source.vertices.empty()) continue;
		if (source.vertices.size() > std::numeric_limits<UINT>::max() ||
			source.vertices.size() > std::numeric_limits<UINT>::max() / sizeof(ObjVertex)) {
			error = "OBJ mesh is too large for a Direct3D vertex buffer.";
			return false;
		}
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = static_cast<UINT>(source.vertices.size() * sizeof(ObjVertex));
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA init{};
		init.pSysMem = source.vertices.data();
		GpuMesh mesh;
		if (FAILED(m_device->CreateBuffer(&desc, &init, mesh.vertexBuffer.GetAddressOf()))) {
			error = "Could not create a Direct3D vertex buffer for the OBJ model.";
			return false;
		}
		mesh.vertexCount = static_cast<UINT>(source.vertices.size());
		mesh.materialName = source.materialName;
		mesh.groupName = source.groupName;
		meshes.push_back(std::move(mesh));
	}
	if (meshes.empty()) {
		error = "OBJ has no renderable triangles.";
		return false;
	}

	float minBounds[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	float maxBounds[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
	std::unordered_map<std::string, std::array<float, 6>> groupBounds;
	for (const auto& mesh : model.meshes) {
		auto group = groupBounds.end();
		if (!mesh.groupName.empty()) {
			const std::array<float, 6> initial{
				std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
				-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()
			};
			group = groupBounds.try_emplace(mesh.groupName, initial).first;
		}
		for (const auto& vertex : mesh.vertices) {
			for (size_t axis = 0; axis < 3; ++axis) {
				minBounds[axis] = std::min(minBounds[axis], vertex.position[axis]);
				maxBounds[axis] = std::max(maxBounds[axis], vertex.position[axis]);
				if (group != groupBounds.end()) {
					group->second[axis] = std::min(group->second[axis], vertex.position[axis]);
					group->second[axis + 3] = std::max(group->second[axis + 3], vertex.position[axis]);
				}
			}
		}
	}
	for (size_t axis = 0; axis < 3; ++axis) m_center[axis] = (minBounds[axis] + maxBounds[axis]) * 0.5f;
	const float dx = maxBounds[0] - m_center[0];
	const float dy = maxBounds[1] - m_center[1];
	const float dz = maxBounds[2] - m_center[2];
	m_radius = std::sqrt(dx * dx + dy * dy + dz * dz);
	if (!std::isfinite(m_radius) || m_radius < 0.0001f) {
		error = "OBJ model has invalid or zero-sized bounds.";
		return false;
	}

	m_meshes = std::move(meshes);
	m_materials = std::move(materials);
	m_groupBounds = std::move(groupBounds);
	return true;
}

bool ObjModelPreview::LoadModel(const std::filesystem::path& path, std::string& error) {
	m_isLoaded = false;
	m_meshes.clear();
	m_materials.clear();
	m_groupBounds.clear();
	m_model = {};
	if (!m_isInitialized) {
		error = "The Direct3D preview renderer is not initialized.";
		return false;
	}
	ObjModel model;
	if (!LoadObjModel(path, model, error)) return false;
	if (!CreateGpuModel(model, error)) return false;
	m_model = std::move(model);
	m_isLoaded = true;
	return true;
}

bool ObjModelPreview::Render(ID3D11DeviceContext* context, float rotationRadians,
							 const std::vector<ObjPreviewLayer>& layers, const ObjPreviewColors& colors,
							 const ObjPreviewCamera* camera) {
	if (!context || !m_isInitialized || !m_isLoaded || !m_previewRtv || !m_previewSrv) return false;
	if (layers.empty()) return false;

	ObjPreviewCamera defaultCam{};
	const ObjPreviewCamera& cam = camera ? *camera : defaultCam;

	float minBounds[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	float maxBounds[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
	bool foundBounds = false;

	if (!cam.referenceGroup.empty()) {
		const auto refIt = m_groupBounds.find(cam.referenceGroup);
		if (refIt != m_groupBounds.end()) {
			for (size_t axis = 0; axis < 3; ++axis) {
				minBounds[axis] = refIt->second[axis];
				maxBounds[axis] = refIt->second[axis + 3];
			}
			foundBounds = true;
		}
	}

	if (!foundBounds) {
		// Lock to the first valid layer (typically the base body) so adding/removing armor pieces never shifts the camera
		for (const auto& layer : layers) {
			const auto bounds = m_groupBounds.find(layer.groupName);
			if (bounds != m_groupBounds.end()) {
				for (size_t axis = 0; axis < 3; ++axis) {
					minBounds[axis] = bounds->second[axis];
					maxBounds[axis] = bounds->second[axis + 3];
				}
				foundBounds = true;
				break;
			}
		}
	}
	if (!foundBounds) return false;

	float center[3]{};
	for (size_t axis = 0; axis < 3; ++axis) center[axis] = (minBounds[axis] + maxBounds[axis]) * 0.5f;
	const float dx = maxBounds[0] - center[0];
	const float dy = maxBounds[1] - center[1];
	const float dz = maxBounds[2] - center[2];
	const float radius = std::sqrt(dx * dx + dy * dy + dz * dz);
	if (!std::isfinite(radius) || radius < 0.0001f) return false;

	ComPtr<ID3D11RenderTargetView> oldRtv;
	ComPtr<ID3D11DepthStencilView> oldDsv;
	context->OMGetRenderTargets(1, oldRtv.GetAddressOf(), oldDsv.GetAddressOf());
	UINT viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT oldViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	context->RSGetViewports(&viewportCount, oldViewports);

	ID3D11RenderTargetView* target = m_previewRtv.Get();
	context->OMSetRenderTargets(1, &target, m_depthDsv.Get());
	const D3D11_VIEWPORT viewport{0.0f, 0.0f, static_cast<float>(m_rtWidth), static_cast<float>(m_rtHeight), 0.0f, 1.0f};
	context->RSSetViewports(1, &viewport);
	const float clear[4] = {
		0.0f, 0.0f, 0.0f,
		cam.transparentBackground ? 0.0f : 1.0f
	};
	context->ClearRenderTargetView(target, clear);
	context->ClearDepthStencilView(m_depthDsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	const float zoom = std::clamp(cam.zoom, 0.30f, 2.8f);
	// The camera **strafes** with focusXOffset (eye and focus move together), so the subject actually shifts across the
	// frame - aiming only the focus barely moved it (6.7% between the two shoulder views, measured 2026-10-31).
	const XMVECTOR eye = XMVectorSet(cam.focusXOffset, 0.06f, -3.2f / zoom, 1.0f);
	const XMVECTOR focus = XMVectorSet(cam.focusXOffset, cam.focusYOffset, 0.0f, 1.0f);
	const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);
	const float aspect = static_cast<float>(m_rtWidth) / static_cast<float>(m_rtHeight);
	const XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(30.0f), aspect, 0.05f, 100.0f);
	const float uniformScale = 1.72f / radius;
	const XMMATRIX world = XMMatrixTranslation(-center[0], -center[1], -center[2]) *
						   XMMatrixRotationY(rotationRadians) *
						   XMMatrixRotationX(cam.pitchRadians) *
						   XMMatrixScaling(uniformScale, uniformScale, uniformScale);

	PreviewConstants constants{};
	XMStoreFloat4x4(&constants.worldViewProjection, XMMatrixTranspose(world * view * projection));
	XMStoreFloat4x4(&constants.world, XMMatrixTranspose(world));
	constants.primaryColor = XMFLOAT4(colors.primary[0], colors.primary[1], colors.primary[2], colors.primary[3]);
	constants.secondaryColor = XMFLOAT4(colors.secondary[0], colors.secondary[1], colors.secondary[2], colors.secondary[3]);
	constants.detailColor = XMFLOAT4(colors.detail[0], colors.detail[1], colors.detail[2], colors.detail[3]);
	constants.visorColor = XMFLOAT4(colors.visor[0], colors.visor[1], colors.visor[2], colors.visor[3]);

	UINT stride = sizeof(ObjVertex);
	UINT offset = 0;
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
	context->OMSetDepthStencilState(m_depthState.Get(), 0);
	context->RSSetState(m_rasterizerState.Get());

	for (const auto& layer : layers) {
		for (const auto& mesh : m_meshes) {
			if (mesh.groupName != layer.groupName) continue;
			if (!layer.includedMaterials.empty() &&
				std::find(layer.includedMaterials.begin(), layer.includedMaterials.end(), mesh.materialName) == layer.includedMaterials.end()) continue;
			if (std::find(layer.excludedMaterials.begin(), layer.excludedMaterials.end(), mesh.materialName) != layer.excludedMaterials.end()) continue;
			if (ShouldSkipDecalCardMaterial(mesh.materialName)) continue;

			const ID3D11Buffer* vertexBuffer = mesh.vertexBuffer.Get();
			ID3D11Buffer* boundVertexBuffer = const_cast<ID3D11Buffer*>(vertexBuffer);
			context->IASetVertexBuffers(0, 1, &boundVertexBuffer, &stride, &offset);
			constants.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			ID3D11ShaderResourceView* texture = nullptr;
			ID3D11ShaderResourceView* colorControlTexture = nullptr;
			ID3D11ShaderResourceView* normalTexture = nullptr;
			auto material = m_materials.find(mesh.materialName);
			if (material != m_materials.end()) {
				std::copy(std::begin(material->second.diffuse), std::end(material->second.diffuse), &constants.diffuse.x);
				if (!material->second.textureLoadAttempted && !material->second.diffuseTexturePath.empty()) {
					material->second.textureLoadAttempted = true;
					CreateTexture(material->second.diffuseTexturePath, material->second.diffuseTexture.GetAddressOf());
				}
				if (!material->second.colorControlLoadAttempted && !material->second.colorControlTexturePath.empty()) {
					material->second.colorControlLoadAttempted = true;
					CreateTexture(material->second.colorControlTexturePath, material->second.colorControlTexture.GetAddressOf());
				}
				if (!material->second.normalLoadAttempted && !material->second.normalTexturePath.empty()) {
					material->second.normalLoadAttempted = true;
					CreateTexture(material->second.normalTexturePath, material->second.normalTexture.GetAddressOf());
				}
				texture = material->second.diffuseTexture.Get();
				colorControlTexture = material->second.colorControlTexture.Get();
				normalTexture = material->second.normalTexture.Get();
			}

			const int mode = ClassifyMaterialMode(mesh.materialName, colorControlTexture != nullptr, layer.useSecondaryColor);
			constants.options = XMFLOAT4(
				texture ? 1.0f : 0.0f,
				colorControlTexture ? 1.0f : 0.0f,
				normalTexture ? 1.0f : 0.0f,
				static_cast<float>(mode)
			);
			context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &constants, 0, 0);
			ID3D11Buffer* cb = m_constantBuffer.Get();
			context->VSSetConstantBuffers(0, 1, &cb);
			context->PSSetConstantBuffers(0, 1, &cb);
			ID3D11ShaderResourceView* textures[] = {texture, colorControlTexture, normalTexture};
			context->PSSetShaderResources(0, ARRAYSIZE(textures), textures);
			context->Draw(mesh.vertexCount, 0);
		}
	}
	ID3D11ShaderResourceView* nullTextures[3]{};
	context->PSSetShaderResources(0, ARRAYSIZE(nullTextures), nullTextures);

	// Put back whatever was bound before. With no previous target (Noble's own menu frames) the count must be 0:
	// OMSetRenderTargets(1, nullptr, ...) reads through the null array and crashes inside d3d11.dll.
	ID3D11RenderTargetView* restoreRtv = oldRtv.Get();
	context->OMSetRenderTargets(oldRtv ? 1u : 0u, oldRtv ? &restoreRtv : nullptr, oldDsv.Get());
	if (viewportCount > 0) context->RSSetViewports(viewportCount, oldViewports);
	return true;
}

} // namespace noble

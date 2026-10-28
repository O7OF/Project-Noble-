#include "ObjModelPreview.hpp"

#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <objbase.h>
#include <wincodec.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <limits>
#include <wrl/client.h>

namespace noble {
namespace {

using Microsoft::WRL::ComPtr;
using namespace DirectX;

constexpr UINT kPreviewSize = 512;

struct alignas(16) PreviewConstants {
	XMFLOAT4X4 worldViewProjection;
	XMFLOAT4X4 world;
	XMFLOAT4 diffuse;
	XMFLOAT4 options;
};

static_assert(sizeof(PreviewConstants) % 16 == 0);

const char* kShaderSource = R"(
cbuffer PreviewConstants : register(b0)
{
	float4x4 g_worldViewProjection;
	float4x4 g_world;
	float4 g_diffuse;
	float4 g_options;
};

Texture2D g_diffuseTexture : register(t0);
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
};

VSOutput VSMain(VSInput input)
{
	VSOutput output;
	output.position = mul(float4(input.position, 1.0f), g_worldViewProjection);
	output.normal = normalize(mul(float4(input.normal, 0.0f), g_world).xyz);
	output.texcoord = input.texcoord;
	return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
	float3 lightDirection = normalize(float3(-0.45f, 0.75f, -0.5f));
	float lighting = 0.28f + 0.72f * saturate(dot(normalize(input.normal), lightDirection));
	float4 albedo = g_diffuse;
	if (g_options.x > 0.5f)
		albedo *= g_diffuseTexture.Sample(g_sampler, input.texcoord);
	return float4(albedo.rgb * lighting, albedo.a);
}
)";

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
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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
	colorDesc.Width = kPreviewSize;
	colorDesc.Height = kPreviewSize;
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
	depthDesc.Width = kPreviewSize;
	depthDesc.Height = kPreviewSize;
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

bool ObjModelPreview::Render(ID3D11DeviceContext* context, float rotationRadians, std::string_view visibleGroup) {
	if (!context || !m_isInitialized || !m_isLoaded || !m_previewRtv || !m_previewSrv) return false;
	float center[3] = {m_center[0], m_center[1], m_center[2]};
	float radius = m_radius;
	if (!visibleGroup.empty()) {
		const auto bounds = m_groupBounds.find(std::string(visibleGroup));
		if (bounds == m_groupBounds.end()) return false;
		for (size_t axis = 0; axis < 3; ++axis) {
			center[axis] = (bounds->second[axis] + bounds->second[axis + 3]) * 0.5f;
		}
		const float dx = bounds->second[3] - center[0];
		const float dy = bounds->second[4] - center[1];
		const float dz = bounds->second[5] - center[2];
		radius = std::sqrt(dx * dx + dy * dy + dz * dz);
		if (!std::isfinite(radius) || radius < 0.0001f) return false;
	}

	ComPtr<ID3D11RenderTargetView> oldRtv;
	ComPtr<ID3D11DepthStencilView> oldDsv;
	context->OMGetRenderTargets(1, oldRtv.GetAddressOf(), oldDsv.GetAddressOf());
	UINT viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT oldViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	context->RSGetViewports(&viewportCount, oldViewports);

	ID3D11RenderTargetView* target = m_previewRtv.Get();
	context->OMSetRenderTargets(1, &target, m_depthDsv.Get());
	const D3D11_VIEWPORT viewport{0.0f, 0.0f, static_cast<float>(kPreviewSize), static_cast<float>(kPreviewSize), 0.0f, 1.0f};
	context->RSSetViewports(1, &viewport);
	const float clear[4] = {0.035f, 0.045f, 0.065f, 1.0f};
	context->ClearRenderTargetView(target, clear);
	context->ClearDepthStencilView(m_depthDsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	const XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -3.2f, 1.0f);
	const XMVECTOR focus = XMVectorZero();
	const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);
	const XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(35.0f), 1.0f, 0.05f, 100.0f);
	const XMMATRIX world = XMMatrixTranslation(-center[0], -center[1], -center[2]) *
						   XMMatrixRotationY(rotationRadians) * XMMatrixScaling(1.7f / radius, 1.7f / radius, 1.7f / radius);
	PreviewConstants constants{};
	XMStoreFloat4x4(&constants.worldViewProjection, XMMatrixTranspose(world * view * projection));
	XMStoreFloat4x4(&constants.world, XMMatrixTranspose(world));

	UINT stride = sizeof(ObjVertex);
	UINT offset = 0;
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
	context->OMSetDepthStencilState(m_depthState.Get(), 0);
	context->RSSetState(m_rasterizerState.Get());

	for (size_t i = 0; i < m_meshes.size(); ++i) {
		const auto& mesh = m_meshes[i];
		if (!visibleGroup.empty() && mesh.groupName != visibleGroup) continue;
		const ID3D11Buffer* vertexBuffer = mesh.vertexBuffer.Get();
		ID3D11Buffer* boundVertexBuffer = const_cast<ID3D11Buffer*>(vertexBuffer);
		context->IASetVertexBuffers(0, 1, &boundVertexBuffer, &stride, &offset);
		constants.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		ID3D11ShaderResourceView* texture = nullptr;
		auto material = m_materials.find(mesh.materialName);
		if (material != m_materials.end()) {
			std::copy(std::begin(material->second.diffuse), std::end(material->second.diffuse), &constants.diffuse.x);
			if (!material->second.textureLoadAttempted && !material->second.diffuseTexturePath.empty()) {
				material->second.textureLoadAttempted = true;
				CreateTexture(material->second.diffuseTexturePath, material->second.diffuseTexture.GetAddressOf());
			}
			texture = material->second.diffuseTexture.Get();
		}
		constants.options = XMFLOAT4(texture ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
		context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &constants, 0, 0);
		ID3D11Buffer* cb = m_constantBuffer.Get();
		context->VSSetConstantBuffers(0, 1, &cb);
		context->PSSetConstantBuffers(0, 1, &cb);
		context->PSSetShaderResources(0, 1, &texture);
		context->Draw(mesh.vertexCount, 0);
	}
	ID3D11ShaderResourceView* nullTexture = nullptr;
	context->PSSetShaderResources(0, 1, &nullTexture);

	ID3D11RenderTargetView* restoreRtv = oldRtv.Get();
	context->OMSetRenderTargets(1, oldRtv ? &restoreRtv : nullptr, oldDsv.Get());
	if (viewportCount > 0) context->RSSetViewports(viewportCount, oldViewports);
	return true;
}

} // namespace noble

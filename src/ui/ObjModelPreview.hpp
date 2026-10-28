#pragma once

#include "ObjModel.hpp"

#include <d3d11.h>
#include <wrl/client.h>
#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace noble {

struct ObjPreviewLayer {
	std::string groupName;
	std::vector<std::string> includedMaterials;
	std::vector<std::string> excludedMaterials;
};

struct ObjPreviewColors {
	std::array<float, 4> primary{1.0f, 1.0f, 1.0f, 1.0f};
	std::array<float, 4> secondary{1.0f, 1.0f, 1.0f, 1.0f};
	std::array<float, 4> detail{1.0f, 1.0f, 1.0f, 1.0f};
	std::array<float, 4> visor{1.0f, 1.0f, 1.0f, 1.0f};
};

class ObjModelPreview {
public:
	bool Initialize(ID3D11Device* device);
	bool LoadModel(const std::filesystem::path& path, std::string& error);
	bool Render(ID3D11DeviceContext* context, float rotationRadians,
			const std::vector<ObjPreviewLayer>& layers, const ObjPreviewColors& colors);

	ID3D11ShaderResourceView* GetPreviewTexture() const { return m_previewSrv.Get(); }
	bool IsLoaded() const { return m_isLoaded; }
	const ObjModel& GetModel() const { return m_model; }

private:
	struct GpuMesh {
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		UINT vertexCount = 0;
		std::string materialName;
		std::string groupName;
	};

	struct GpuMaterial {
		float diffuse[4]{1.0f, 1.0f, 1.0f, 1.0f};
		std::filesystem::path diffuseTexturePath;
		std::filesystem::path colorControlTexturePath;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> colorControlTexture;
		bool textureLoadAttempted = false;
		bool colorControlLoadAttempted = false;
	};

	bool CreatePipeline();
	bool CreatePreviewTarget();
	bool CreateTexture(const std::filesystem::path& path, ID3D11ShaderResourceView** texture);
	bool CreateGpuModel(const ObjModel& model, std::string& error);

	ID3D11Device* m_device = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_previewTexture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_previewRtv;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_previewSrv;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDsv;

	ObjModel m_model;
	std::vector<GpuMesh> m_meshes;
	std::unordered_map<std::string, GpuMaterial> m_materials;
	std::unordered_map<std::string, std::array<float, 6>> m_groupBounds;
	float m_center[3]{};
	float m_radius = 1.0f;
	bool m_isInitialized = false;
	bool m_isLoaded = false;
};

} // namespace noble

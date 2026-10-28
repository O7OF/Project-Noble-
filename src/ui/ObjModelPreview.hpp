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
	bool useSecondaryColor = false;
};

struct ObjPreviewColors {
	std::array<float, 4> primary{1.0f, 1.0f, 1.0f, 1.0f};
	std::array<float, 4> secondary{1.0f, 1.0f, 1.0f, 1.0f};
	std::array<float, 4> detail{0.16f, 0.17f, 0.19f, 1.0f};
	std::array<float, 4> visor{0.85f, 0.56f, 0.14f, 1.0f};
};

struct ObjPreviewCamera {
	float pitchRadians = 0.04f;
	float zoom = 1.42f;            // 1.0 = full body, ~1.42 = MCC 3/4 portrait framing
	float focusYOffset = 0.14f;    // Shifts look-at target upward toward upper torso/head
	float focusXOffset = 0.0f;
	std::string referenceGroup;    // Locks camera center/radius to base body group
	bool transparentBackground = true;
};

class ObjModelPreview {
public:
	bool Initialize(ID3D11Device* device);
	bool LoadModel(const std::filesystem::path& path, std::string& error);
	bool Render(ID3D11DeviceContext* context, float rotationRadians,
			const std::vector<ObjPreviewLayer>& layers, const ObjPreviewColors& colors,
			const ObjPreviewCamera* camera = nullptr);

	ID3D11ShaderResourceView* GetPreviewTexture() const { return m_previewSrv.Get(); }
	bool IsLoaded() const { return m_isLoaded; }
	const ObjModel& GetModel() const { return m_model; }
	UINT GetWidth() const { return m_rtWidth; }
	UINT GetHeight() const { return m_rtHeight; }

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
		std::filesystem::path normalTexturePath;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> colorControlTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalTexture;
		bool textureLoadAttempted = false;
		bool colorControlLoadAttempted = false;
		bool normalLoadAttempted = false;
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
	UINT m_rtWidth = 960;
	UINT m_rtHeight = 1080;
	bool m_isInitialized = false;
	bool m_isLoaded = false;
};

} // namespace noble

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace noble {

struct ObjVertex {
	float position[3]{};
	float normal[3]{};
	float texcoord[2]{};
};

struct ObjMaterial {
	std::string name;
	float diffuse[4]{1.0f, 1.0f, 1.0f, 1.0f};
	std::filesystem::path diffuseTexture;
};

struct ObjMesh {
	std::string objectName;
	std::string groupName;
	std::string materialName;
	std::vector<ObjVertex> vertices;
};

struct ObjModel {
	std::vector<ObjMaterial> materials;
	std::vector<ObjMesh> meshes;
};

bool LoadObjModel(const std::filesystem::path& path, ObjModel& model, std::string& error);

} // namespace noble

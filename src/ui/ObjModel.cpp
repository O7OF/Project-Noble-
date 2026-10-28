#include "ObjModel.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace noble {
namespace {

constexpr size_t kMaxLineLength = 1024 * 1024;
constexpr size_t kMaxElements = 2'000'000;

struct FaceIndex {
	int position = 0;
	int texcoord = 0;
	int normal = 0;
};

bool IsSafeRelativePath(const std::filesystem::path& path) {
	if (path.empty() || path.is_absolute() || path.has_root_name()) return false;
	for (const auto& part : path) {
		if (part == "..") return false;
	}
	return true;
}

std::filesystem::path ResolveRelative(const std::filesystem::path& base, const std::string& value) {
	const std::filesystem::path relative = std::filesystem::u8path(value);
	if (!IsSafeRelativePath(relative)) return {};
	return (base / relative).lexically_normal();
}

std::filesystem::path FindTextureFile(const std::filesystem::path& candidate) {
	const std::array<std::filesystem::path, 6> extensions{
		candidate.extension(), ".png", ".tif", ".tiff", ".jpg", ".jpeg"
	};
	for (const auto& extension : extensions) {
		std::filesystem::path path = candidate;
		path.replace_extension(extension);
		std::error_code ec;
		if (std::filesystem::is_regular_file(path, ec)) return path;
	}
	return {};
}

std::filesystem::path ResolveTexturePath(const std::filesystem::path& mtlDirectory,
										 const std::filesystem::path& modelDirectory,
										 const std::string& value) {
	const std::filesystem::path relative = std::filesystem::u8path(value);
	if (!IsSafeRelativePath(relative)) return {};

	std::filesystem::path resolved = FindTextureFile((mtlDirectory / relative).lexically_normal());
	if (!resolved.empty()) return resolved;

	std::filesystem::path bitmapRelative;
	bool afterBitmaps = false;
	for (const auto& part : relative) {
		std::string name = part.string();
		std::transform(name.begin(), name.end(), name.begin(), [](unsigned char ch) {
			return static_cast<char>(std::tolower(ch));
		});
		if (afterBitmaps) {
			bitmapRelative /= part;
		} else if (name == "bitmaps") {
			afterBitmaps = true;
		}
	}
	if (afterBitmaps && !bitmapRelative.empty()) {
		resolved = FindTextureFile((modelDirectory / "bitmaps" / bitmapRelative).lexically_normal());
		if (!resolved.empty()) return resolved;
	}

	return FindTextureFile(modelDirectory / "bitmaps" / relative.filename());
}

std::filesystem::path FindColorControlTexture(const std::filesystem::path& diffuseTexture) {
	if (diffuseTexture.empty()) return {};
	std::string stem = diffuseTexture.stem().string();
	std::string lowered = stem;
	std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});

	size_t suffix = lowered.rfind("_diffuse");
	size_t suffixLength = 8;
	if (suffix == std::string::npos) {
		suffix = lowered.rfind("_diff");
		suffixLength = 5;
	}
	if (suffix == std::string::npos) return {};

	stem.replace(suffix, suffixLength, "_cc");
	const auto candidate = diffuseTexture.parent_path() / (stem + diffuseTexture.extension().string());
	std::error_code ec;
	return std::filesystem::is_regular_file(candidate, ec) ? candidate : std::filesystem::path{};
}

bool ParseFloat(std::string_view text, float& value) {
	try {
		size_t used = 0;
		value = std::stof(std::string(text), &used);
		return used == text.size() && std::isfinite(value);
	} catch (...) {
		return false;
	}
}

bool ParseInt(std::string_view text, int& value) {
	try {
		size_t used = 0;
		value = std::stoi(std::string(text), &used);
		return used == text.size() && value != 0;
	} catch (...) {
		return false;
	}
}

bool ParseFaceIndex(const std::string& token, FaceIndex& index) {
	const size_t firstSlash = token.find('/');
	const size_t secondSlash = firstSlash == std::string::npos ? std::string::npos : token.find('/', firstSlash + 1);

	const std::string_view position(token.data(), firstSlash == std::string::npos ? token.size() : firstSlash);
	if (!ParseInt(position, index.position)) return false;

	if (firstSlash != std::string::npos) {
		const size_t texEnd = secondSlash == std::string::npos ? token.size() : secondSlash;
		const std::string_view texcoord(token.data() + firstSlash + 1, texEnd - firstSlash - 1);
		if (!texcoord.empty() && !ParseInt(texcoord, index.texcoord)) return false;

		if (secondSlash != std::string::npos) {
			const std::string_view normal(token.data() + secondSlash + 1, token.size() - secondSlash - 1);
			if (!normal.empty() && !ParseInt(normal, index.normal)) return false;
		}
	}
	return true;
}

bool ResolveIndex(int rawIndex, size_t count, size_t& resolved) {
	if (rawIndex > 0) {
		const size_t index = static_cast<size_t>(rawIndex - 1);
		if (index >= count) return false;
		resolved = index;
		return true;
	}
	const int64_t index = static_cast<int64_t>(count) + rawIndex;
	if (index < 0 || static_cast<uint64_t>(index) >= count) return false;
	resolved = static_cast<size_t>(index);
	return true;
}

std::array<float, 3> FaceNormal(const float* a, const float* b, const float* c) {
	const float abx = b[0] - a[0];
	const float aby = b[1] - a[1];
	const float abz = b[2] - a[2];
	const float acx = c[0] - a[0];
	const float acy = c[1] - a[1];
	const float acz = c[2] - a[2];
	std::array<float, 3> normal{
		aby * acz - abz * acy,
		abz * acx - abx * acz,
		abx * acy - aby * acx
	};
	const float length = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	if (length > 0.000001f) {
		for (float& component : normal) component /= length;
	} else {
		normal = {0.0f, 1.0f, 0.0f};
	}
	return normal;
}

bool LoadMaterials(const std::filesystem::path& path, const std::filesystem::path& modelDirectory,
				   std::unordered_map<std::string, ObjMaterial>& materials, std::string& error) {
	const auto relative = path.lexically_relative(modelDirectory);
	if (!IsSafeRelativePath(relative)) {
		error = "Material library path escapes the model directory.";
		return false;
	}

	std::ifstream file(path);
	if (!file) {
		error = "Could not open material library: " + path.string();
		return false;
	}

	ObjMaterial* current = nullptr;
	std::string line;
	size_t lineNumber = 0;
	while (std::getline(file, line)) {
		++lineNumber;
		if (line.size() > kMaxLineLength) {
			error = "Material library line is too long at line " + std::to_string(lineNumber) + ".";
			return false;
		}
		std::istringstream input(line);
		std::string command;
		if (!(input >> command) || command[0] == '#') continue;
		if (command == "newmtl") {
			std::string name;
			std::getline(input >> std::ws, name);
			if (name.empty()) continue;
			auto [it, inserted] = materials.try_emplace(name);
			current = &it->second;
			current->name = name;
			(void)inserted;
		} else if (current && command == "Kd") {
			if (!(input >> current->diffuse[0] >> current->diffuse[1] >> current->diffuse[2]) ||
				!std::isfinite(current->diffuse[0]) || !std::isfinite(current->diffuse[1]) ||
				!std::isfinite(current->diffuse[2])) {
				error = "Invalid Kd value at material line " + std::to_string(lineNumber) + ".";
				return false;
			}
		} else if (current && (command == "d" || command == "Tr")) {
			float alpha = 1.0f;
			if (!(input >> alpha) || !std::isfinite(alpha)) {
				error = "Invalid transparency value at material line " + std::to_string(lineNumber) + ".";
				return false;
			}
			current->diffuse[3] = std::clamp(command == "Tr" ? 1.0f - alpha : alpha, 0.0f, 1.0f);
		} else if (current && command == "map_Kd") {
			std::string filename;
			std::getline(input >> std::ws, filename);
			if (!filename.empty() && filename[0] != '-') {
				const auto relative = std::filesystem::u8path(filename);
				if (!IsSafeRelativePath(relative)) {
					error = "Diffuse texture path escapes the model directory.";
					return false;
				}
				const auto texture = ResolveTexturePath(path.parent_path(), modelDirectory, filename);
				current->diffuseTexture = texture;
				current->colorControlTexture = FindColorControlTexture(texture);
			}
		}
	}
	return true;
}

} // namespace

bool LoadObjModel(const std::filesystem::path& path, ObjModel& model, std::string& error) {
	ObjModel parsed;
	const std::filesystem::path modelPath = std::filesystem::absolute(path).lexically_normal();
	const std::filesystem::path modelDirectory = modelPath.parent_path();
	std::ifstream file(modelPath);
	if (!file) {
		error = "Could not open OBJ model: " + modelPath.string();
		return false;
	}

	std::vector<std::array<float, 3>> positions;
	std::vector<std::array<float, 3>> normals;
	std::vector<std::array<float, 2>> texcoords;
	std::unordered_map<std::string, ObjMaterial> materials;
	std::string objectName = "default";
	std::string groupName = "default";
	std::string materialName;
	ObjMesh* currentMesh = nullptr;
	auto ensureMesh = [&]() -> ObjMesh& {
		if (!currentMesh || currentMesh->objectName != objectName || currentMesh->groupName != groupName ||
			currentMesh->materialName != materialName) {
			parsed.meshes.push_back({objectName, groupName, materialName, {}});
			currentMesh = &parsed.meshes.back();
		}
		return *currentMesh;
	};

	std::string line;
	size_t lineNumber = 0;
	while (std::getline(file, line)) {
		++lineNumber;
		if (line.size() > kMaxLineLength) {
			error = "OBJ line is too long at line " + std::to_string(lineNumber) + ".";
			return false;
		}
		std::istringstream input(line);
		std::string command;
		if (!(input >> command) || command[0] == '#') continue;

		if (command == "v") {
			std::array<float, 3> value{};
			if (!(input >> value[0] >> value[1] >> value[2]) ||
				!std::isfinite(value[0]) || !std::isfinite(value[1]) || !std::isfinite(value[2])) {
				error = "Invalid vertex at OBJ line " + std::to_string(lineNumber) + ".";
				return false;
			}
			positions.push_back(value);
		} else if (command == "vn") {
			std::array<float, 3> value{};
			if (!(input >> value[0] >> value[1] >> value[2]) ||
				!std::isfinite(value[0]) || !std::isfinite(value[1]) || !std::isfinite(value[2])) {
				error = "Invalid normal at OBJ line " + std::to_string(lineNumber) + ".";
				return false;
			}
			normals.push_back(value);
		} else if (command == "vt") {
			std::array<float, 2> value{};
			if (!(input >> value[0] >> value[1]) || !std::isfinite(value[0]) || !std::isfinite(value[1])) {
				error = "Invalid texture coordinate at OBJ line " + std::to_string(lineNumber) + ".";
				return false;
			}
			texcoords.push_back(value);
		} else if (command == "mtllib") {
			std::string library;
			while (input >> library) {
				const auto libraryPath = ResolveRelative(modelDirectory, library);
				if (libraryPath.empty() || !LoadMaterials(libraryPath, modelDirectory, materials, error)) {
					if (error.empty()) error = "Invalid material library path at OBJ line " + std::to_string(lineNumber) + ".";
					return false;
				}
			}
		} else if (command == "o" || command == "g") {
			std::string name;
			std::getline(input >> std::ws, name);
			if (name.empty()) name = "default";
			if (command == "o") objectName = name;
			else groupName = name;
			currentMesh = nullptr;
		} else if (command == "usemtl") {
			std::getline(input >> std::ws, materialName);
			currentMesh = nullptr;
		} else if (command == "f") {
			std::vector<FaceIndex> face;
			std::string token;
			while (input >> token) {
				if (!token.empty() && token[0] == '#') break;
				FaceIndex index;
				if (!ParseFaceIndex(token, index)) {
					error = "Invalid face index at OBJ line " + std::to_string(lineNumber) + ".";
					return false;
				}
				face.push_back(index);
			}
			if (face.size() < 3) {
				error = "Face has fewer than three vertices at OBJ line " + std::to_string(lineNumber) + ".";
				return false;
			}

			for (size_t triangle = 1; triangle + 1 < face.size(); ++triangle) {
				const FaceIndex triangleIndices[] = {face[0], face[triangle], face[triangle + 1]};
				std::array<size_t, 3> positionIndices{};
				std::array<size_t, 3> normalIndices{};
				std::array<size_t, 3> texcoordIndices{};
				for (size_t i = 0; i < 3; ++i) {
					if (!ResolveIndex(triangleIndices[i].position, positions.size(), positionIndices[i]) ||
						(triangleIndices[i].normal && !ResolveIndex(triangleIndices[i].normal, normals.size(), normalIndices[i])) ||
						(triangleIndices[i].texcoord && !ResolveIndex(triangleIndices[i].texcoord, texcoords.size(), texcoordIndices[i]))) {
						error = "Face index is out of range at OBJ line " + std::to_string(lineNumber) + ".";
						return false;
					}
				}

				ObjMesh& mesh = ensureMesh();
				if (mesh.vertices.size() > kMaxElements - 3) {
					error = "OBJ contains too many triangle vertices.";
					return false;
				}
				const auto generatedNormal = FaceNormal(positions[positionIndices[0]].data(),
														positions[positionIndices[1]].data(),
														positions[positionIndices[2]].data());
				for (size_t i = 0; i < 3; ++i) {
					ObjVertex vertex{};
					std::copy(positions[positionIndices[i]].begin(), positions[positionIndices[i]].end(), vertex.position);
					if (triangleIndices[i].normal) {
						std::copy(normals[normalIndices[i]].begin(), normals[normalIndices[i]].end(), vertex.normal);
					} else {
						std::copy(generatedNormal.begin(), generatedNormal.end(), vertex.normal);
					}
					if (triangleIndices[i].texcoord) {
						std::copy(texcoords[texcoordIndices[i]].begin(), texcoords[texcoordIndices[i]].end(), vertex.texcoord);
					}
					mesh.vertices.push_back(vertex);
				}
			}
		}

		if (positions.size() > kMaxElements || normals.size() > kMaxElements || texcoords.size() > kMaxElements) {
			error = "OBJ contains too many source elements.";
			return false;
		}
	}

	if (parsed.meshes.empty()) {
		error = "OBJ contains no faces.";
		return false;
	}
	for (auto& [name, material] : materials) {
		(void)name;
		parsed.materials.push_back(std::move(material));
	}
	model = std::move(parsed);
	error.clear();
	return true;
}

} // namespace noble

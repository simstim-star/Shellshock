#pragma once

#include "model.h"
#include <optional>
#include <string_view>

struct ID3D11Device;
struct aiScene;
struct aiMaterial;

class AssetLoader {
  public:
	AssetLoader() : _device{nullptr} {};
	AssetLoader(ID3D11Device *device) : _device{device} {};
	std::optional<Model> LoadModel(std::string_view path);

  private:
	ID3D11Device *_device;

	MeshBuffer LoadMesh(const aiMesh *mesh);
	Material LoadMaterial(const aiScene *scene, const aiMaterial *mat, std::string_view basePath);
	ComPtr<ID3D11ShaderResourceView> LoadMaterialTexture(const aiScene *scene, const char *texPath, std::string_view basePath);
};

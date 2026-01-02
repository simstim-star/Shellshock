#include "asset_loader.h"
#include "log.h"
#include "texture.h"
#include "utility.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <format>

using Microsoft::WRL::ComPtr;

/* Forward declaration of private functions */

static Mesh LoadMesh(ID3D11Device *device, const aiMesh *mesh);
static Material LoadMaterial(ID3D11Device *device, const aiScene *scene, const aiMaterial *mat, std::string_view basePath);
static ComPtr<ID3D11ShaderResourceView>
LoadMaterialTexture(ID3D11Device *device, const aiScene *scene, aiString aiTexPath, std::string_view basePath);
static std::optional<aiString> TexturePath(const aiMaterial *mat, aiTextureType type);

/* Implementation of public functions */

std::optional<Model> AssetLoader::LoadModel(ID3D11Device *device, std::string_view path)
{
	Assimp::Importer importer;
	const aiScene *scene =
		importer.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);

	if (!scene || !scene->HasMeshes()) {
		std::string errorMsg = std::format("Failed to load model asset.\n\nPath: {}\nError: {}", path, importer.GetErrorString());
		TGW::Logger::LogInfo(errorMsg);
		return {};
	}

	std::filesystem::path fsPath{path};
	std::string basePath = fsPath.parent_path().string();

	Model model;
	model.name = fsPath.filename().string();
	model.position = DirectX::XMFLOAT3{0, 0, 0};
	for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
		model.materials.push_back(LoadMaterial(device, scene, scene->mMaterials[i], basePath));
	}

	for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
		model.meshes.push_back(LoadMesh(device, scene->mMeshes[i]));
	}

	return model;
}

/* Implementation of private functions */

Mesh LoadMesh(ID3D11Device *device, const aiMesh *mesh)
{
	std::vector<Vertex> vertices(mesh->mNumVertices);
	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
		vertices[i] = {
		  {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z},
		  {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z},
		  {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}};
	}

	std::vector<uint32_t> indices(mesh->mNumFaces * 3);
	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		indices[i * 3 + 0] = mesh->mFaces[i].mIndices[0];
		indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}

	Mesh out{};
	out.indexCount = (uint32_t)indices.size();
	out.materialIndex = mesh->mMaterialIndex;

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = UINT(sizeof(Vertex) * vertices.size());
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {vertices.data()};
	ASSERT_SUCCEEDED(device->CreateBuffer(&vbDesc, &vbData, out.vertexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = UINT(sizeof(uint32_t) * indices.size());
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {indices.data()};
	ASSERT_SUCCEEDED(device->CreateBuffer(&ibDesc, &ibData, out.indexBuffer.GetAddressOf()));

	return out;
}

ComPtr<ID3D11ShaderResourceView> LoadMaterialTexture(ID3D11Device *device, const aiScene *scene, aiString aiTexPath, std::string_view basePath)
{
	if (const aiTexture *embeddedTex = scene->GetEmbeddedTexture(aiTexPath.C_Str())) {
		const BOOL isCompressed = embeddedTex->mHeight == 0;
		if (isCompressed) {
			return TGW::Texture::LoadEmbeddedCompressed(
				device, reinterpret_cast<const uint8_t *>(embeddedTex->pcData), embeddedTex->mWidth);
		} else {
			// TODO
		}
	}

	std::filesystem::path fullPath = std::filesystem::path(basePath) / aiTexPath.C_Str();
	return TGW::Texture::Load(device, fullPath.wstring().c_str());
}

Material LoadMaterial(ID3D11Device *device, const aiScene *scene, const aiMaterial *mat, std::string_view basePath)
{
	Material m{};
	if (auto diffusePath = TexturePath(mat, aiTextureType_DIFFUSE)) {
		m.diffuse = LoadMaterialTexture(device, scene, diffusePath.value(), basePath);
	}
	if (auto specularPath = TexturePath(mat, aiTextureType_SPECULAR)) {
		m.specular = LoadMaterialTexture(device, scene, specularPath.value(), basePath);
	}
	if (auto normalPath = TexturePath(mat, aiTextureType_HEIGHT)) {
		m.normal = LoadMaterialTexture(device, scene, normalPath.value(), basePath);
	}
	if (auto roughnessPath = TexturePath(mat, aiTextureType_AMBIENT)) {
		m.roughness = LoadMaterialTexture(device, scene, roughnessPath.value(), basePath);
	}
	return m;
}

std::optional<aiString> TexturePath(const aiMaterial *mat, aiTextureType type)
{
	aiString aiTexPath;
	if (mat->GetTexture(type, 0, &aiTexPath) == aiReturn_SUCCESS) {
		return aiTexPath;
	}
	return std::nullopt;
}
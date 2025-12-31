#include "asset_loader.h"
#include "texture_wic.h"
#include "utility.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <format>

using Microsoft::WRL::ComPtr;

/* Forward declaration of private functions */

static Mesh LoadMesh(ID3D11Device *device, const aiMesh *mesh);
static Material LoadMaterial(ID3D11Device *device, const aiMaterial *mat, std::string_view basePath);
static ComPtr<ID3D11ShaderResourceView>
LoadMaterialTexture(ID3D11Device *device, const aiMaterial *mat, aiTextureType type, std::string_view basePath);

/* Implementation of public functions */

std::optional<Model> AssetLoader::LoadModel(ID3D11Device *device, std::string_view path)
{
	Assimp::Importer importer;
	const aiScene *scene =
		importer.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);

	if (!scene || !scene->HasMeshes()) {
		// TODO: Change to Imgui log error later
		std::string errorMsg = std::format("Failed to load model asset.\n\nPath: {}\nError: {}", path, importer.GetErrorString());
		std::wstring wError(errorMsg.begin(), errorMsg.end());
		MessageBox(nullptr, wError.c_str(), L"Asset Loading Error", MB_OK | MB_ICONERROR);
		return {};
	}

	std::string basePath = std::filesystem::path{path}.parent_path().string();

	Model model;
	for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
		model.materials.push_back(LoadMaterial(device, scene->mMaterials[i], basePath));
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

	std::vector<uint32_t> indices;
	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		aiFace &f = mesh->mFaces[i];
		indices.push_back(f.mIndices[0]);
		indices.push_back(f.mIndices[1]);
		indices.push_back(f.mIndices[2]);
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

ComPtr<ID3D11ShaderResourceView>
LoadMaterialTexture(ID3D11Device *device, const aiMaterial *mat, aiTextureType type, std::string_view basePath)
{
	if (mat->GetTextureCount(type) > 0) {
		aiString aiTexPath;
		if (mat->GetTexture(type, 0, &aiTexPath) == AI_SUCCESS) {
			std::filesystem::path fullPath = std::filesystem::path(basePath) / aiTexPath.C_Str();
			return LoadTextureWIC(device, fullPath.wstring().c_str());
		}
	}
	return nullptr;
}

Material LoadMaterial(ID3D11Device *device, const aiMaterial *mat, std::string_view basePath)
{
	Material m{};
	m.diffuse = LoadMaterialTexture(device, mat, aiTextureType_DIFFUSE, basePath);
	m.specular = LoadMaterialTexture(device, mat, aiTextureType_SPECULAR, basePath);
	m.normal = LoadMaterialTexture(device, mat, aiTextureType_HEIGHT, basePath);
	m.roughness = LoadMaterialTexture(device, mat, aiTextureType_AMBIENT, basePath);
	return m;
}
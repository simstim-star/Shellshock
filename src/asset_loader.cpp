#include "asset_loader.h"
#include "log.h"
#include "texture.h"

#include <assimp/importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using Microsoft::WRL::ComPtr;
DirectX::XMMATRIX ConvertToDirectXMatrix(aiMatrix4x4 from);

/* Implementation of public functions */

std::optional<Model> AssetLoader::LoadModel(std::string_view path)
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
	model.worldMatrix = ConvertToDirectXMatrix(scene->mRootNode->mTransformation);
	for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
		model.materials.push_back(LoadMaterial(scene, scene->mMaterials[i], basePath));
	}

	for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
		model.meshes.push_back(LoadMesh(scene->mMeshes[i]));
	}

	return model;
}

/* Implementation of private functions */

MeshBuffer AssetLoader::LoadMesh(const aiMesh *mesh)
{
	std::vector<Vertex> vertices(mesh->mNumVertices);
	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
		vertices[i] = {
		  {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z},
		  {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z},
		  {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}};
	}
	D3D11_SUBRESOURCE_DATA vbData = {vertices.data()};

	std::vector<uint32_t> indices(mesh->mNumFaces * 3);
	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		indices[i * 3 + 0] = mesh->mFaces[i].mIndices[0];
		indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
		indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
	}
	D3D11_SUBRESOURCE_DATA ibData = {indices.data()};

	MeshBuffer out{
	  .indexCount = (uint32_t)indices.size(),
	  .materialIndex = mesh->mMaterialIndex,
	};

	D3D11_BUFFER_DESC vbDesc{
	  .ByteWidth = UINT(sizeof(Vertex) * vertices.size()),
	  .Usage = D3D11_USAGE_DEFAULT,
	  .BindFlags = D3D11_BIND_VERTEX_BUFFER,
	};
	ASSERT_SUCCEEDED(_device->CreateBuffer(&vbDesc, &vbData, out.vertexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC ibDesc{
	  .ByteWidth = UINT(sizeof(uint32_t) * indices.size()),
	  .Usage = D3D11_USAGE_DEFAULT,
	  .BindFlags = D3D11_BIND_INDEX_BUFFER,
	};
	ASSERT_SUCCEEDED(_device->CreateBuffer(&ibDesc, &ibData, out.indexBuffer.GetAddressOf()));

	return out;
}

ComPtr<ID3D11ShaderResourceView>
AssetLoader::LoadMaterialTexture(const aiScene *scene, const char *texPath, std::string_view basePath)
{
	if (const aiTexture *embeddedTex = scene->GetEmbeddedTexture(texPath)) {
		const BOOL isCompressed = embeddedTex->mHeight == 0;
		if (isCompressed) {
			return TGW::Texture::LoadEmbeddedCompressed(
				_device, reinterpret_cast<const uint8_t *>(embeddedTex->pcData), embeddedTex->mWidth);
		} else {
			// TODO
		}
	}

	std::filesystem::path fullPath = std::filesystem::path(basePath) / texPath;
	return TGW::Texture::Load(_device, fullPath.wstring().c_str());
}

Material AssetLoader::LoadMaterial(const aiScene *scene, const aiMaterial *mat, std::string_view basePath)
{
	Material m{};
	aiString aiTexPath;

	if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == aiReturn_SUCCESS) {
		m.diffuse = LoadMaterialTexture(scene, aiTexPath.C_Str(), basePath);
	}

	if (mat->GetTexture(aiTextureType_SPECULAR, 0, &aiTexPath) == aiReturn_SUCCESS) {
		m.specular = LoadMaterialTexture(scene, aiTexPath.C_Str(), basePath);
	}

	if (mat->GetTexture(aiTextureType_HEIGHT, 0, &aiTexPath) == aiReturn_SUCCESS) {
		m.normal = LoadMaterialTexture(scene, aiTexPath.C_Str(), basePath);
	}

	if (mat->GetTexture(aiTextureType_AMBIENT, 0, &aiTexPath) == aiReturn_SUCCESS) {
		m.roughness = LoadMaterialTexture(scene, aiTexPath.C_Str(), basePath);
	}

	return m;
}

DirectX::XMMATRIX ConvertToDirectXMatrix(aiMatrix4x4 from)
{
	return DirectX::XMMATRIX(
		from.a1, from.a2, from.a3, from.a4, from.b1, from.b2, from.b3, from.b4, from.c1, from.c2, from.c3, from.c4, from.d1,
		from.d2, from.d3, from.d4);
}
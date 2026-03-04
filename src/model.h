#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct aiMesh;

struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoords;
};

struct MeshBuffer {
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	uint32_t indexCount = 0;
	uint32_t materialIndex = 0;
};

struct Material {
	ComPtr<ID3D11ShaderResourceView> diffuse;
	ComPtr<ID3D11ShaderResourceView> specular;
	ComPtr<ID3D11ShaderResourceView> roughness;
	ComPtr<ID3D11ShaderResourceView> normal;
};

struct Model {
	std::string name;
	UINT id;
	DirectX::XMMATRIX worldMatrix;
	std::vector<MeshBuffer> meshes;
	std::vector<Material> materials;
};
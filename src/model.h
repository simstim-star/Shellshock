#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <memory>
#include <string_view>
#include <string>
#include <vector>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct Vertex {
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexCoords;
};

struct Mesh {
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
	std::vector<Mesh> meshes;
	std::vector<Material> materials;
};
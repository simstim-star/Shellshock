#pragma once

#include <d3dcompiler.h>
#include <windows.h>
#include <DirectXMath.h>

struct ConstantBuffer {
	DirectX::XMMATRIX model;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;

	DirectX::XMFLOAT3 cameraPos;
	float isSelected{0.0f};
};

HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob **blob);
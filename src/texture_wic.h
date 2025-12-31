#pragma once

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
struct ID3D11ShaderResourceView;
struct ID3D11Device;

// TODO: Not very COM-like, consider changing later
ComPtr<ID3D11ShaderResourceView> LoadTextureWIC(
    ID3D11Device* device,
    const wchar_t* filename
);

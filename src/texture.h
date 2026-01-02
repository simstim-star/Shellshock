#pragma once

#include <windows.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
struct ID3D11ShaderResourceView;
struct ID3D11Device;

namespace TGW::Texture {
ComPtr<ID3D11ShaderResourceView> Load(ID3D11Device *device, const WCHAR *filename);
ComPtr<ID3D11ShaderResourceView> LoadEmbeddedCompressed(ID3D11Device *device, const UINT8 *data, const size_t dataSize);
} // namespace TGW::Texture

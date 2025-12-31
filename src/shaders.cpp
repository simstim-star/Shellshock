#include "shaders.h"
#include "utility.h"

#pragma comment(lib, "d3dcompiler.lib")

HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob **blob)
{
	if (!srcFile || !entryPoint || !profile || !blob) {
		return E_INVALIDARG;
	}

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *shaderBlob = nullptr;
	ID3DBlob *errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(
		srcFile, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			ERROR((char *)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		if (shaderBlob) {
			shaderBlob->Release();
		}
		return hr;
	}

	*blob = shaderBlob;

	return hr;
}
#include "texture_wic.h"

#include <d3d11.h>
#include <wincodec.h>

#include "utility.h"
#include <cstdint>
#include <stdexcept>
#include <vector>

#define WIC_CHECK_SUCESS(hr)   \
	ASSERT_SUCCEEDED(hr);      \
	if (FAILED(hr)) {          \
		return nullptr;        \
	}

ComPtr<ID3D11ShaderResourceView> LoadTextureWIC(ID3D11Device *device, const wchar_t *filename)
{
	if (!device || !filename) {
		return nullptr;
	}

	ComPtr<IWICImagingFactory> factory;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
	WIC_CHECK_SUCESS(hr);

	ComPtr<IWICBitmapDecoder> decoder;
	hr = factory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	WIC_CHECK_SUCESS(hr);

	ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, &frame);
	WIC_CHECK_SUCESS(hr);

	ComPtr<IWICFormatConverter> converter;
	hr = factory->CreateFormatConverter(&converter);
	WIC_CHECK_SUCESS(hr);

	hr = converter->Initialize(
		frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
	WIC_CHECK_SUCESS(hr);

	UINT width = 0, height = 0;
	converter->GetSize(&width, &height);

	std::vector<uint8_t> pixels(width * height * 4);
	hr = converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(pixels.size()), pixels.data());
	WIC_CHECK_SUCESS(hr);

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA init{};
	init.pSysMem = pixels.data();
	init.SysMemPitch = width * 4;

	ComPtr<ID3D11Texture2D> texture;
	hr = device->CreateTexture2D(&desc, &init, texture.GetAddressOf());
	WIC_CHECK_SUCESS(hr);

	ComPtr<ID3D11ShaderResourceView> srv;
	hr = device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf());
	WIC_CHECK_SUCESS(hr);
	return srv;
}

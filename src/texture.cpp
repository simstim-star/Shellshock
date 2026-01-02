#include "texture.h"
#include "DDSTextureLoader.h"
#include "log.h"
#include "utility.h"
#include <WICTextureLoader.h>
#include <assimp/scene.h>
#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <wincodec.h>
#include <wrl/client.h>

#define WIC_CHECK_SUCESS(hr)  \
	ASSERT_SUCCEEDED(hr);     \
	if (FAILED(hr)) {         \
		return nullptr;       \
	}

static ComPtr<ID3D11ShaderResourceView> LoadWIC(ID3D11Device *device, const WCHAR *filename);

ComPtr<ID3D11ShaderResourceView> TGW::Texture::Load(ID3D11Device *device, const WCHAR *filename)
{
	std::wstring extension = std::filesystem::path{filename}.extension().wstring();
	for (auto &character : extension) {
		character = std::tolower(character);
	}

	if (extension == L".dds") {
		ComPtr<ID3D11ShaderResourceView> srv;
		HRESULT hr = DirectX::CreateDDSTextureFromFile(device, filename, nullptr, srv.GetAddressOf());
		if (SUCCEEDED(hr)) {
			return srv;
		} else {
			std::filesystem::path p{filename};
			const std::string info =
				std::format("Failed to load DDS texture: {} | HRESULT: 0x{:08X}", p.string(), static_cast<unsigned int>(hr));
			Logger::LogInfo(info);
		}
	}
	return LoadWIC(device, filename);
}

ComPtr<ID3D11ShaderResourceView>
TGW::Texture::LoadEmbeddedCompressed(ID3D11Device *device, const UINT8 *data, const size_t dataSize)
{
	if (!device) {
		return nullptr;
	}

	ComPtr<ID3D11ShaderResourceView> srv = nullptr;
	HRESULT hr = DirectX::CreateWICTextureFromMemory(device, data, dataSize, nullptr, &srv);
	if (FAILED(hr)) {
		const std::string info =
			std::format("Failed to load embedded texture. HRESULT: 0x{:08X}", static_cast<unsigned int>(hr));
		Logger::LogInfo(info);
	}

	return srv;
}

// TODO: Not very COM-like, consider changing later
ComPtr<ID3D11ShaderResourceView> LoadWIC(ID3D11Device *device, const WCHAR *filename)
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

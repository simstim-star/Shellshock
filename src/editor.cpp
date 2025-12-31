#include "editor.h"
#include "texture_wic.h"

#include "shaders.h"
#include "utility.h"
#include <variant>

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

constexpr float CLEAR_COLOR[] = {0.1f, 0.2f, 0.6f, 1.0f};

void TGW::Editor::Init(HWND hwnd)
{
	_hwnd = hwnd;

	RECT rc;
	GetClientRect(hwnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferCount = 2;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.BufferDesc.Width = width;
	scd.BufferDesc.Height = height;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hwnd;
	scd.SampleDesc.Count = 1;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ASSERT_SUCCEEDED(D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &scd, &_swapchain, &_device,
		nullptr, &_context));

	ID3D11Texture2D *backbuffer;
	ASSERT_SUCCEEDED(_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer));
	ASSERT_SUCCEEDED(_device->CreateRenderTargetView(backbuffer, nullptr, &_rtv));
	backbuffer->Release();

	D3D11_VIEWPORT vp = {};
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	_context->RSSetViewports(1, &vp);

	float aspect = vp.Width / vp.Height;

	_matView = XMMatrixLookAtLH(XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	_matProj = XMMatrixPerspectiveFovLH(_camera.GetAngle(), aspect, 0.1f, 100.0f);

	D3D11_TEXTURE2D_DESC dtd = {};
	dtd.Width = width;
	dtd.Height = height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dtd.SampleDesc.Count = 1;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> depthBuffer;
	ASSERT_SUCCEEDED(_device->CreateTexture2D(&dtd, nullptr, depthBuffer.GetAddressOf()));
	ASSERT_SUCCEEDED(_device->CreateDepthStencilView(depthBuffer.Get(), nullptr, &_dsv));

	TGW::GUI::Init(hwnd, _device.Get(), _context.Get());
	_gui = std::make_unique<TGW::GUI::EditorMain>(_uiCommandQueue);

	LoadAssets();
}

void TGW::Editor::Render()
{
	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX mvp = world * _matView * _matProj;
	XMMATRIX mvpTransposed = XMMatrixTranspose(mvp);
	_context->UpdateSubresource(_cbMVP.Get(), 0, nullptr, &mvpTransposed, 0, 0);

	_context->VSSetConstantBuffers(0, 1, _cbMVP.GetAddressOf());
	_context->RSSetState(_rasterState.Get());
	_context->OMSetRenderTargets(1, _rtv.GetAddressOf(), _dsv.Get());
	_context->ClearRenderTargetView(_rtv.Get(), CLEAR_COLOR);
	_context->ClearDepthStencilView(_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	_context->IASetInputLayout(_inputLayout.Get());
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_context->VSSetShader(_vs.Get(), nullptr, 0);
	_context->PSSetShader(_ps.Get(), nullptr, 0);

	_context->PSSetSamplers(0, 1, _sampler.GetAddressOf());

	for (Model &model : _models) {
		DrawModel(model);
	}
	_gui->Render();

	ASSERT_SUCCEEDED(_swapchain->Present(1, 0));
}

void TGW::Editor::DrawModel(const Model &model)
{
	for (const auto &mesh : model.meshes) {
		const Material &material = model.materials[mesh.materialIndex];
		ID3D11ShaderResourceView *srvs[4] = {
		  material.diffuse ? material.diffuse.Get() : nullptr, material.specular ? material.specular.Get() : nullptr,
		  material.normal ? material.normal.Get() : nullptr, material.roughness ? material.roughness.Get() : nullptr};
		_context->PSSetShaderResources(0, 4, srvs);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
		_context->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		_context->DrawIndexed(mesh.indexCount, 0, 0);
	}
}

void TGW::Editor::Update()
{
	_camera.HandleMouse(_hwnd);
	_matView = _camera.LookAt();
	_gui->Update();
	ProcessUICommands();
}

void TGW::Editor::LoadAssets()
{
	ID3DBlob *vsBlob = nullptr;
	ID3DBlob *psBlob = nullptr;
	ASSERT_SUCCEEDED(CompileShader(L"shaders/textured.hlsl", "VSMain", "vs_5_0", &vsBlob));
	ASSERT_SUCCEEDED(CompileShader(L"shaders/textured.hlsl", "PSMain", "ps_5_0", &psBlob));
	ASSERT_SUCCEEDED(_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &_vs));
	ASSERT_SUCCEEDED(_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &_ps));

	D3D11_INPUT_ELEMENT_DESC layout[] = {
	  {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	  {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	  {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}};
	ASSERT_SUCCEEDED(_device->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &_inputLayout));

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = sampDesc.AddressV = sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ASSERT_SUCCEEDED(_device->CreateSamplerState(&sampDesc, &_sampler));

	D3D11_BUFFER_DESC cbd = {};
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.ByteWidth = sizeof(XMMATRIX);
	cbd.Usage = D3D11_USAGE_DEFAULT;
	ASSERT_SUCCEEDED(_device->CreateBuffer(&cbd, nullptr, &_cbMVP));

	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.FrontCounterClockwise = false;
	ASSERT_SUCCEEDED(_device->CreateRasterizerState(&rasterDesc, &_rasterState));
}

// TODO: simplify this...
void TGW::Editor::ProcessUICommands()
{
	for (const auto &cmd : _uiCommandQueue) {
		std::visit(
			[this](auto &&arg) {
				using T = std::decay_t<decltype(arg)>;

				if constexpr (std::is_same_v<T, LoadModelCommand>) {
					auto newModel = AssetLoader::LoadModel(_device.Get(), arg.path);
					if (newModel) {
						_models.clear();
						_models.push_back(std::move(newModel.value()));
					}
				}
			},
			cmd);
	}
	_uiCommandQueue.clear(); // Clear for the next frame
}
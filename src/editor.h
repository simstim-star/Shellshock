#pragma once

#include "asset_loader.h"
#include <d3d11.h>
#include <wrl/client.h>

#include "camera.h"
#include "gui/gui.h"
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

namespace TGW {

class Editor {
  public:
	Editor(HINSTANCE hInstance);
	void Run(int nCmdShow);
	void Resize(UINT width, UINT height);
	void Render();
	void DrawModel(const Model &model);
	void Update();

	inline void HandleZoom(float wheelDelta) { _camera.HandleZoom(wheelDelta); }

  private:
	void LoadAssets();
	void CreateGUI();

	HWND _hwnd;

	ComPtr<ID3D11Device> _device;
	ComPtr<ID3D11DeviceContext> _context;
	ComPtr<IDXGISwapChain> _swapchain;
	ComPtr<ID3D11RenderTargetView> _rtv;
	ComPtr<ID3D11VertexShader> _vs;
	ComPtr<ID3D11PixelShader> _ps;
	ComPtr<ID3D11InputLayout> _inputLayout;
	ComPtr<ID3D11SamplerState> _sampler;
	ComPtr<ID3D11DepthStencilView> _dsv;
	ComPtr<ID3D11Buffer> _cbMVP;
	ComPtr<ID3D11RasterizerState> _rasterState;
	ComPtr<ID3D11RasterizerState> _rasterStateOutline;

	std::unordered_map<UINT, Model> _models;

	DirectX::XMMATRIX _matView;
	DirectX::XMMATRIX _matProj;
	Camera _camera;

	std::unique_ptr<GUI::MainUI> _gui;

	AssetLoader _assetLoader;

	std::optional<UINT> _selectedModel = std::nullopt;
};
} // namespace TGW
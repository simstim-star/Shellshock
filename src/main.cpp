#include "editor.h"
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TGW::Editor *editor = reinterpret_cast<TGW::Editor *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (msg) {
	case WM_NCCREATE: {
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		return TRUE;
	}
	case WM_MOUSEWHEEL:
		if (editor) {
			short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			editor->GetCamera().HandleZoom(wheelDelta);
		}
		return FALSE;
	case WM_PAINT:
		if (editor) {
			editor->Update();
			editor->Render();
		}
		return FALSE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return FALSE;
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		return -1;
	}

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"TGW";
	RegisterClass(&wc);

	TGW::Editor editor;
	HWND hwnd = CreateWindowEx(
		0, wc.lpszClassName, L"TGW", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance,
		&editor);
	editor.Init(hwnd);
	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

#include "gui.h"

#include "ImGuiFileDialog.h"
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

constexpr auto CHOOSE_MODEL_DIALOG_KEY = "ChooseModelKey";

TGW::GUI::Base::~Base()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void TGW::GUI::Init(HWND hwnd, ID3D11Device *device, ID3D11DeviceContext *context)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, context);
}

void TGW::GUI::Base::Update()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void TGW::GUI::Base::Render()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void TGW::GUI::EditorMain::Update()
{
	Base::Update();

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open Model...")) {
				IGFD::FileDialogConfig config;
				config.path = ".";
				config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ShowDevicesButton;
				ImGuiFileDialog::Instance()->OpenDialog(CHOOSE_MODEL_DIALOG_KEY, "Select Model", ".obj,.fbx,.gltf", config);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (ImGuiFileDialog::Instance()->Display(CHOOSE_MODEL_DIALOG_KEY)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
				_queue.push_back(LoadModelCommand{path});
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
}
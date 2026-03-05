#include "gui.h"

#include <d3d11.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include "ImGuizmo.h"
#include "ImGuiFileDialog.h"

#include <log.h>
/* Consts */

constexpr auto MASTER_DOCKSPACE_ID = "MasterDockspace";
constexpr auto CHOOSE_MODEL_DIALOG_KEY = "ChooseModelKey";
constexpr auto LOG_WHITE = ImVec4(1, 1, 1, 1);
constexpr std::array<ImVec4, static_cast<size_t>(TGW::LogType::NUM_LOG_TYPES)> LOG_COLORS = {
  LOG_WHITE,
};

#define LOG_COLOR(log) LOG_COLORS.at(static_cast<size_t>(log.type))

/* Class implementations */

TGW::GUI::MainUI::~MainUI()
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
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, context);
}

void TGW::GUI::MainUI::Render()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void TGW::GUI::MainUI::Update(const EditorMetadata &editorMetadata)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	GenerateDockspace();

	UpdateTopMenu();
	UpdateLogs();
	UpdateAssets(editorMetadata);

	ImGui::End();
}

void TGW::GUI::MainUI::GenerateDockspace()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;
	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	windowFlags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	// Note: this window is persisted! You need to delete the imgui.ini in the build folder to clean the cache and see changes
	ImGui::Begin("MasterDockspaceWindow", nullptr, windowFlags);
	ImGui::PopStyleVar(3);

	ImGuiID masterDockspaceID = ImGui::GetID(MASTER_DOCKSPACE_ID);

	// This only runs ONCE, the master dock doesn't need to be recreated
	// (unless the .ini is removed)
	if (!ImGui::DockBuilderGetNode(masterDockspaceID)) {
		ImGui::DockBuilderAddNode(masterDockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(masterDockspaceID, viewport->Size);

		// Create a bottom dockspace at the bottom dockSplitPercentage
		ImGuiID mainDockID = masterDockspaceID;
		float dockSplitPercentage = 0.25f;
		ImGuiID bottomDockID =
			ImGui::DockBuilderSplitNode(masterDockspaceID, ImGuiDir_Down, dockSplitPercentage, nullptr, &mainDockID);

		ImGui::DockBuilderDockWindow("Logs", bottomDockID);
		ImGui::DockBuilderDockWindow("Assets", bottomDockID);
		ImGui::DockBuilderFinish(masterDockspaceID);
	}

	ImGui::DockSpace(masterDockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
}

void TGW::GUI::MainUI::UpdateTopMenu()
{
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			// Note: ImGui short cut in menu item is display only: https://github.com/ocornut/imgui/issues/6381
			if (ImGui::MenuItem("Open Model...", "Ctrl+O")) {
				IGFD::FileDialogConfig config;
				config.path = ".";
				ImGuiFileDialog::Instance()->OpenDialog(CHOOSE_MODEL_DIALOG_KEY, "Choose Model", ".obj,.fbx,.gltf,.glb", config);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGuiFileDialog::Instance()->Display(CHOOSE_MODEL_DIALOG_KEY)) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			_OnLoadModel(filePathName);
			Logger::LogInfo("Loading Model " + filePathName);
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void TGW::GUI::MainUI::UpdateLogs()
{
	if (ImGui::Begin("Logs")) {
		if (ImGui::Button("Clear")) {
			TGW::Logger::Clear();
		}
		ImGui::SameLine();
		bool copyToClipboard = ImGui::Button("Copy All");
		ImGui::Separator();
		ImGui::BeginChild("LogScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		std::string fullLog;
		for (const auto &entry : TGW::Logger::GetAll()) {
			std::string logLine = entry.AsString();
			ImGui::TextColored(LOG_COLOR(entry), logLine.c_str());
			if (copyToClipboard) {
				fullLog += logLine + "\n";
			}
		}

		if (copyToClipboard) {
			ImGui::SetClipboardText(fullLog.c_str());
		}

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
	}
	ImGui::End();
}

void TGW::GUI::MainUI::UpdateAssets(const EditorMetadata &editorMetadata)
{
	if (ImGui::Begin("Assets")) {
		static char assetFilter[64] = "";
		ImGui::InputTextWithHint("##AssetFilter", "Search assets...", assetFilter, IM_ARRAYSIZE(assetFilter));
		ImGui::Separator();

		if (ImGui::BeginTable("AssetsTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			for (const auto &asset : editorMetadata.assets) {
				if (assetFilter[0] != '\0' && asset.name.find(assetFilter) == std::string::npos)
					continue;

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				bool selected = false; // TODO: track this in EditorMetadata
				if (ImGui::Selectable(asset.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
					_OnSelectModel(asset.id);
				}
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::Button("Remove Model")) {
						_OnRemoveModel(asset.id);
					}
					ImGui::EndPopup();
				}
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void TGW::GUI::MainUI::UpdateGizmo(Model &model, const Camera &camera)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGuizmo::BeginFrame();
	ImGuizmo::AllowAxisFlip(false);
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

	float vMatrix[16];
	DirectX::XMMATRIX view = camera.GetViewMatrix();
	DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4 *>(vMatrix), view);

	float pMatrix[16];
	DirectX::XMMATRIX proj = camera.GetProjectionMatrix();
	DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4 *>(pMatrix), proj);

	float wMatrix[16];
	DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4 *>(wMatrix), model.worldMatrix);

	ImGuizmo::Manipulate(vMatrix, pMatrix, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, wMatrix);

	if (ImGuizmo::IsUsing()) {
		model.worldMatrix = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4 *>(wMatrix));
	}
}
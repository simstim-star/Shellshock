#pragma once

#include "../metadata.h"
#include <d3d11.h>
#include <vector>
#include <windows.h>
#include <queue>
#include <functional>

#include "camera.h"
#include "model.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace TGW::GUI {
void Init(HWND hwnd, ID3D11Device *device, ID3D11DeviceContext *context);

class MainUI {
  public:
	MainUI(
		std::function<void(std::string)> OnLoadModel, std::function<void(UINT id)> OnSelectModel,
		std::function<void(UINT id)> OnRemoveModel)
		: _OnLoadModel{OnLoadModel}, _OnSelectModel{OnSelectModel}, _OnRemoveModel{OnRemoveModel}
	{
	}
	
	~MainUI();

	void Render();

	void GenerateDockspace();
	
	void Update(const EditorMetadata &editorMetadata);
	void UpdateTopMenu();
	void UpdateGizmo(Model &model, const Camera &camera);

  private:
	void UpdateLogs();
	void UpdateAssets(const EditorMetadata &editorMetadata);
	std::function<void(std::string)> _OnLoadModel;
	std::function<void(UINT id)> _OnSelectModel;
	std::function<void(UINT id)> _OnRemoveModel;
};
} // namespace TGW::GUI

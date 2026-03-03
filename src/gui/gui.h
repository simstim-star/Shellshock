#pragma once

#include "../metadata.h"
#include <d3d11.h>
#include <vector>
#include <windows.h>
#include <queue>
#include <functional>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace TGW::GUI {
void Init(HWND hwnd, ID3D11Device *device, ID3D11DeviceContext *context);

class Base {
  public:
	~Base();
	virtual void Render();
	virtual void Update(const EditorMetadata &editorMetadata);
};

class EditorMain : public Base {
  public:
	EditorMain(std::function<void(std::string)> OnLoadModel, std::function<void(UINT id)> OnSelectModel)
		: _OnLoadModel{OnLoadModel}, _OnSelectModel{OnSelectModel}
	{
	}
	void Update(const EditorMetadata &editorMetadata) override;

	void GenerateDockspace();

	void UpdateTopMenu();

  private:
	void UpdateLogs();
	void UpdateAssets(const EditorMetadata &editorMetadata);
	std::function<void(std::string)> _OnLoadModel;
	std::function<void(UINT id)> _OnSelectModel;
};
} // namespace TGW::GUI

#pragma once

#include "../actions.h"
#include "../metadata.h"
#include <d3d11.h>
#include <vector>
#include <windows.h>

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
	EditorMain(std::vector<EditorCommand> &commandQueue) : _queue(commandQueue) {}
	void Update(const EditorMetadata &editorMetadata) override;

	void GenerateDockspace();

	void UpdateTopMenu();

  private:
	void UpdateLogs();
	void UpdateAssets(const EditorMetadata &editorMetadata);
	std::vector<EditorCommand> &_queue;
};
} // namespace TGW::GUI

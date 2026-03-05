#pragma once

#include "pch.h"

namespace TGW::GUI {

struct AssetMetadata {
	UINT id;
	std::string name;
};

struct EditorMetadata {
	std::vector<AssetMetadata> assets;
};

} // namespace TGW::GUI
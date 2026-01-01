#pragma once

#include <string>
#include <vector>

namespace TGW::GUI {
struct AssetMetadata {
	std::string name;
};

struct EditorMetadata {
	std::vector<AssetMetadata> assets;
};
} // namespace TGW::GUI
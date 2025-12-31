#pragma once

#include <string>
#include <variant>
#include <vector>

namespace TGW {
struct LoadModelCommand {
	std::string path;
};

// TODO: simplify this...
using EditorCommand = std::variant<LoadModelCommand>;
}
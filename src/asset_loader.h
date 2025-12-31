#pragma once

#include "model.h"
#include <optional>
#include <string_view>

struct ID3D11Device;

namespace AssetLoader {
std::optional<Model> LoadModel(ID3D11Device *device, std::string_view path);
};

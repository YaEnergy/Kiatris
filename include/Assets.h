#pragma once

#include <unordered_map>
#include <string>

#include "raylib-cpp.hpp"

void LoadAssets();

raylib::Texture2D& GetTexture(std::string name);
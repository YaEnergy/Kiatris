#pragma once

#include <unordered_map>
#include <string>

#include "raylib-cpp.hpp"

void LoadAssets();

void UnloadAssets();

raylib::Texture2D& GetTexture(std::string name);

raylib::Sound& GetSound(std::string name);

raylib::Music& GetMusic(std::string name);
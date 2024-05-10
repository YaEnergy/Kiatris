#include "Assets.h"

std::unordered_map<std::string, raylib::Texture2D> textures;

void LoadAssets()
{
	textures.emplace("BlockPiece", raylib::Texture2D("assets/textures/piece_block.png"));
	textures["BlockPiece"].SetWrap(TEXTURE_WRAP_REPEAT);
}

raylib::Texture2D& GetTexture(std::string name)
{
	return textures[name];
}
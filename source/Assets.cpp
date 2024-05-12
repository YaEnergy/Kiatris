#include "Assets.h"

std::unordered_map<std::string, raylib::Texture2D> textures;
std::unordered_map<std::string, raylib::Sound> sounds;

void LoadAssets()
{
	//textures
	textures.emplace("BlockPiece", raylib::Texture2D("assets/textures/piece_block.png"));
	textures["BlockPiece"].SetWrap(TEXTURE_WRAP_REPEAT);

	//sounds
	sounds.emplace("PlacePiece", raylib::Sound("assets/sfx/piece_place.wav"));
	sounds.emplace("LevelUp", raylib::Sound("assets/sfx/level_up.wav"));
	sounds.emplace("GameOver", raylib::Sound("assets/sfx/lose.wav"));
	sounds.emplace("LineClear", raylib::Sound("assets/sfx/line_clear.wav"));
}

void UnloadAssets()
{
	//iterate over all textures and unload them

	std::unordered_map<std::string, raylib::Texture2D>::iterator textureIt = textures.begin();
	while (textureIt != textures.end())
	{
		textureIt->second.Unload();

		//go to next item
		textureIt++;
	}

	textures.clear();

	//iterate over all sounds and unload them

	std::unordered_map<std::string, raylib::Sound>::iterator soundIt = sounds.begin();
	while (soundIt != sounds.end())
	{
		soundIt->second.Unload();

		//go to next item
		soundIt++;
	}

	sounds.clear();
}

raylib::Texture2D& GetTexture(std::string name)
{
	return textures[name];
}

raylib::Sound& GetSound(std::string name)
{
	return sounds[name];
}
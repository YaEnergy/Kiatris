#include "Assets.h"

std::unordered_map<std::string, raylib::Texture2D> textures;
std::unordered_map<std::string, raylib::Sound> sounds;
std::unordered_map<std::string, raylib::Music> musicFiles;
std::unordered_map<std::string, raylib::Font> fonts;

void LoadAssets()
{
	//textures
	textures.emplace("Icon", raylib::Texture2D("assets/textures/kiatrisicon.png"));
	textures.emplace("BlockPiece", raylib::Texture2D("assets/textures/piece_block.png"));
	GetTexture("BlockPiece").SetWrap(TEXTURE_WRAP_REPEAT);

	//sounds
	sounds.emplace("PlacePiece", raylib::Sound("assets/sfx/piece_place.wav"));
	sounds.emplace("LevelUp", raylib::Sound("assets/sfx/level_up.wav"));
	sounds.emplace("GameOver", raylib::Sound("assets/sfx/lose.wav"));
	sounds.emplace("LineClear", raylib::Sound("assets/sfx/line_clear.wav"));

	//music
	musicFiles.emplace("MainTheme", raylib::Music("assets/music/Nowhere Land.mp3"));
	musicFiles.emplace("MenuTheme", raylib::Music("assets/music/Bleeping Demo.mp3"));

	raylib::Music& mainTheme = GetMusic("MainTheme");
	mainTheme.SetLooping(true);
	mainTheme.SetVolume(0.2f);

	raylib::Music& menuTheme = GetMusic("MenuTheme");
	menuTheme.SetLooping(true);
	menuTheme.SetVolume(0.2f);

	//fonts
	fonts.emplace("MainFont", raylib::Font());
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

	//music is automatically unloaded
}

raylib::Texture2D& GetTexture(std::string name)
{
	return textures.at(name);
}

raylib::Sound& GetSound(std::string name)
{
	return sounds.at(name);
}

raylib::Music& GetMusic(std::string name)
{
	return musicFiles.at(name);
}

raylib::Font& GetFont(std::string name)
{
	return fonts.at(name);
}
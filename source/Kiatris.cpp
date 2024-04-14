// Kiatris.cpp : Defines the entry point for the application.
//

#include "Kiatris.h"
#include "raylib-cpp.hpp"
#include "Game/SceneManager.h"
#include "Game/SceneGame.h"

class Game
{
	private:
		raylib::Window window;
		raylib::AudioDevice audioDevice;
		SceneManager sceneManager;

		void UpdateDrawFrame()
		{
			sceneManager.Update();

			window.BeginDrawing();

			window.ClearBackground(raylib::BLACK);

			sceneManager.Draw();
			window.DrawFPS(10, 10);

			window.EndDrawing();
		}
	public:
		Game() : window(DESIGN_WIDTH, DESIGN_HEIGHT, "Kiatris"), audioDevice()
		{
			SceneGame gameScene(window, { GAMEMODE_ENDLESS, 3, {10, 20}, true});
			sceneManager.SetScene(&gameScene);
			
			window.SetState(FLAG_WINDOW_RESIZABLE);
			//TODO: loading assets, flags, init, Emscripten modifications, audio device

			while (!window.ShouldClose())
			{
				UpdateDrawFrame();
			}

			//window and audio device are closed automatically
		}
};

#if WIN32RELEASE
int WinMain()
{
	Game game;

	return 0;
}
#else
int main()
{
	Game game;

	return 0;
}
#endif
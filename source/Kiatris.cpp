// Kiatris.cpp : Defines the entry point for the application.
//

#include "Kiatris.h"
#include "Assets.h"
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
		Game() : window(DESIGN_WIDTH, DESIGN_HEIGHT, "Kiatris", FLAG_VSYNC_HINT), audioDevice()
		{
			raylib::Image icon = raylib::Image("assets/textures/kiatrisicon.png");
			window.SetIcon(icon);

			SceneGame gameScene(window, { 3, {10, 20}, true});
			sceneManager.SetScene(&gameScene);
			
			window.SetState(FLAG_WINDOW_RESIZABLE);
			
			int monitor = GetCurrentMonitor();
			window.SetMonitor(monitor);
			window.SetTargetFPS(GetMonitorRefreshRate(monitor));

			SetExitKey(KEY_NULL);

			//Loading background
			window.BeginDrawing();
			window.ClearBackground(raylib::Color::Black());

			raylib::DrawText("Loading...", 12, 12, 36, raylib::Color::White());

			raylib::DrawText("Please wait...", 12, 48, 36, raylib::Color::White());

			window.EndDrawing();

			//Load
			LoadAssets();

			//TODO: loading assets, flags, init, Emscripten modifications, audio device

			while (!window.ShouldClose())
			{
				if (GetCurrentMonitor() != monitor)
				{
					monitor = GetCurrentMonitor();
					window.SetTargetFPS(GetMonitorRefreshRate(monitor));
				}

				UpdateDrawFrame();
			}
			
			icon.Unload();
			UnloadAssets();
			//window, audio device are closed and unloaded automatically
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
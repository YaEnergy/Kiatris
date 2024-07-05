// Kiatris.cpp : Defines the entry point for the application.
//

#include "Kiatris.h"
#include "Assets.h"
#include "raylib-cpp.hpp"
#include "Game/SceneGame.h"

#ifdef PLATFORM_WEB
	#include "emscripten.h"
#endif

//args = Game class instance, must be done for Emscripten due to it not acceping C++ methods
void UpdateDrawFrame(void* args);

class Game
{
	private:
		raylib::Window window;
		raylib::AudioDevice audioDevice;
		SceneGame sceneGame;
	public:
		void Update()
		{
			sceneGame.Update();
		}

		void Draw()
		{
			window.BeginDrawing();

			window.ClearBackground(raylib::BLACK);

			sceneGame.Draw();

#if DEBUG
			window.DrawFPS(10, 10);
#endif

			window.EndDrawing();
		}

		Game() : window(DESIGN_WIDTH, DESIGN_HEIGHT, "Kiatris", FLAG_VSYNC_HINT), audioDevice(), sceneGame(window, GameOptions())
		{
			raylib::Image icon = raylib::Image("assets/textures/kiatrisicon.png");
			window.SetIcon(icon);
			
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

#if defined(PLATFORM_WEB)
			DisableCursor();

			emscripten_set_main_loop_arg(UpdateDrawFrame, this, 0, 1);
#else
			HideCursor();

			while (!window.ShouldClose() && !sceneGame.WantsToQuit)
			{
				if (GetCurrentMonitor() != monitor)
				{
					monitor = GetCurrentMonitor();
					window.SetTargetFPS(GetMonitorRefreshRate(monitor));
				}

				UpdateDrawFrame(this);
			}
#endif
			
			icon.Unload();
			UnloadAssets();

			//window, audio device are closed and unloaded automatically
		}
};

#if WIN32RELEASE
int WinMain()
{
	return main();
}
#endif

int main()
{
	Game game;

	return 0;
}

//args = Game class instance, must be done for Emscripten due to it not acceping C++ methods
void UpdateDrawFrame(void* args)
{
	static_cast<Game*>(args)->Update();
	static_cast<Game*>(args)->Draw();
}
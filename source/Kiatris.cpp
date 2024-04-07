// Kiatris.cpp : Defines the entry point for the application.
//

#include "Kiatris.h"

class Game
{
	private:
		raylib::Window window;
		raylib::AudioDevice audioDevice;

		void UpdateDrawFrame()
		{
			window.BeginDrawing();

			window.ClearBackground(raylib::BLACK);

			window.DrawFPS(10, 10);

			window.EndDrawing();
		}
	public:
		Game(raylib::Window window, raylib::AudioDevice audioDevice)
		{
			this->window = window;
			this->audioDevice = audioDevice;

			//TODO: loading assets, flags, init, Emscripten modifications, audio device

			while (!window.ShouldClose())
			{
				UpdateDrawFrame();
			}

			window.Close();
			audioDevice.Close();
		}
};

#if WIN32RELEASE
int WinMain()
{
	Game game(raylib::Window(DESIGN_WIDTH, DESIGN_HEIGHT, "Kiatris"), raylib::AudioDevice());

	return 0;
}
#else
int main()
{
	Game game(raylib::Window(DESIGN_WIDTH, DESIGN_HEIGHT, "Kiatris"), raylib::AudioDevice());

	return 0;
}
#endif
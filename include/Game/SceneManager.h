#pragma once

#include "Scene.h"

class SceneManager
{
	private:
		Scene* currentScene;

	public:
		void SetScene(Scene* scene);

		void Update();

		void Draw();

		SceneManager()
		{
			currentScene = nullptr;
		}

		SceneManager(Scene* scene)
		{
			currentScene = scene;

			SetScene(scene);
		}
};
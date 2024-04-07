#include "Game/SceneManager.h"

void SceneManager::SetScene(Scene* scene)
{
	if (currentScene != nullptr)
		currentScene->Destroy();

	currentScene = scene;

	scene->Init();
}

void SceneManager::Update()
{
	if (currentScene != nullptr)
		currentScene->Update();
}

void SceneManager::Draw()
{
	if (currentScene != nullptr)
		currentScene->Draw();
}
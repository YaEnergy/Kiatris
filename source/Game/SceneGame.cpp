#include "Game/SceneGame.h"

void SceneGame::Init()
{
	
}

void SceneGame::Update()
{

}

void SceneGame::Draw()
{
	
}

void SceneGame::Destroy()
{
	currentPiece.~Piece();
	holdingPiece.~Piece();

	//Destroy up and coming
}
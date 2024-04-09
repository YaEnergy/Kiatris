#include "Game/SceneGame.h"

const raylib::Color EMPTY_BLOCK_COLOR = raylib::Color::Blank();

void SceneGame::Init()
{
	
}

void SceneGame::Update()
{
	if (gameOver)
		UpdateGameOver();
	else
		UpdateGameplay();
}

#pragma region Gameplay

void SceneGame::UpdateGameplay()
{
	//for testing purposes
	Vector2Int movement = { 0, 0 };

	if (IsKeyPressed(KEY_RIGHT))
		movement.x = 1;
	else if (IsKeyPressed(KEY_LEFT))
		movement.x = -1;

	if (IsKeyPressed(KEY_DOWN))
		movement.y = 1;
	else if (IsKeyPressed(KEY_UP))
		movement.y = -1;

	if (IsKeyPressed(KEY_E))
		currentPiece.RotateLeft();
	else if (IsKeyPressed(KEY_R))
		currentPiece.RotateRight();
	else if (IsKeyPressed(KEY_T))
		currentPiece.RotateHalfCircle();

	currentPiecePosition = { currentPiecePosition.x + movement.x, currentPiecePosition.y + movement.y };

	//TODO: getting new pieces causes a memory leak, the pointers for pieces are most likely not being deleted. Fix this!!!!!
	//for (int i = 0; i < 2000; i++)
		//PlacePiece();

	if (IsKeyPressed(KEY_SPACE))
		PlacePiece();
}

void SceneGame::UpdateGameOver()
{

}

void SceneGame::EndGame()
{

}

#pragma endregion

#pragma region Pieces

void SceneGame::PlacePiece()
{
	//Place piece into grid
	for (int i = 0; i < currentPiece.numBlocks; i++)
	{
		if (currentPiecePosition.y + currentPiece.blockOffsets[i].y >= gameModifiers.GridSize.y || currentPiecePosition.y + currentPiece.blockOffsets[i].y < 0)
			continue;

		if (currentPiecePosition.x + currentPiece.blockOffsets[i].x >= gameModifiers.GridSize.x || currentPiecePosition.x + currentPiece.blockOffsets[i].x < 0)
			continue;

		grid[currentPiecePosition.y + currentPiece.blockOffsets[i].y][currentPiecePosition.x + currentPiece.blockOffsets[i].x] = currentPiece.blockColors[i];
	}

	//New piece
	currentPiecePosition = { 0, 0 };
	currentPiece = *(Piece::GetMainPiece((MainPieceType)GetRandomValue(0, 6)));
}

#pragma endregion

void SceneGame::Draw()
{
	//prototype

	Vector2 blockSize = { 32.0f, 32.0f };

	Vector2 totalGridSize = { blockSize.x * gameModifiers.GridSize.x, blockSize.y * gameModifiers.GridSize.y };

	//Draw grid
	for (int y = 0; y < gameModifiers.GridSize.y; y++)
	{
		for (int x = 0; x < gameModifiers.GridSize.x; x++)
		{
			if (grid[y][x] == EMPTY_BLOCK_COLOR)
				continue;

			raylib::Rectangle rect = { blockSize.x * x, blockSize.y * y, blockSize.x, blockSize.y };
			rect.Draw(grid[y][x]);
		}
	}

	//Draw current piece
	for (int i = 0; i < currentPiece.numBlocks; i++)
	{
		raylib::Rectangle rect = { blockSize.x * (currentPiecePosition.x + currentPiece.blockOffsets[i].x), blockSize.y * (currentPiecePosition.y + currentPiece.blockOffsets[i].y), blockSize.x, blockSize.y};
		rect.Draw(currentPiece.blockColors[i]);
	}

	//Draw score
	raylib::DrawText("Score: " + std::to_string(score), totalGridSize.x + 10, 0, 36, raylib::Color::White());
}

void SceneGame::Destroy()
{
	//TODO: finish this
	//add if not null pointer

	//delete currentPiece;
	//delete holdingPiece;

	//Destroy up and coming
	delete[] upAndComingPieces;
}
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

	//movement
	if (IsKeyPressed(KEY_RIGHT))
		movement.x = 1;
	else if (IsKeyPressed(KEY_LEFT))
		movement.x = -1;

	if (IsKeyPressed(KEY_DOWN))
		movement.y = 1;
	else if (IsKeyPressed(KEY_UP))
		movement.y = -1;

	//rotation
	if (IsKeyPressed(KEY_E))
		currentPiece.RotateLeft();
	else if (IsKeyPressed(KEY_R))
		currentPiece.RotateRight();
	else if (IsKeyPressed(KEY_T))
		currentPiece.RotateHalfCircle();

	Vector2Int newPiecePosition = { currentPiecePosition.x + movement.x, currentPiecePosition.y + movement.y };
	if (CanPieceExistAt(newPiecePosition))
		currentPiecePosition = newPiecePosition;
	else
	{
		//backtrack rotation, for testing purposes only rn
		//TODO: improve this
		if (IsKeyPressed(KEY_E))
			currentPiece.RotateRight();
		else if (IsKeyPressed(KEY_R))
			currentPiece.RotateLeft();
		else if (IsKeyPressed(KEY_T))
			currentPiece.RotateHalfCircle();
	}

	if (IsKeyPressed(KEY_SPACE))
		PlacePiece();
	else if (IsKeyPressed(KEY_C))
		HoldPiece();
}

void SceneGame::UpdateGameOver()
{

}

void SceneGame::EndGame()
{

}

#pragma endregion

#pragma region Pieces

Piece SceneGame::GetRandomPiece()
{
	return Piece::GetMainPiece((MainPieceType)GetRandomValue(0, 6));
}

bool SceneGame::CanPieceExistAt(Vector2Int position)
{
	//if any block is out of bounds or not empty, then the piece can not exist at this position
	for (int i = 0; i < currentPiece.numBlocks; i++)
	{
		if (!IsCellEmpty(position.x + currentPiece.blockOffsets[i].x, position.y + currentPiece.blockOffsets[i].y))
			return false;
	}

	return true;
}

void SceneGame::NextPiece()
{
	//get next piece in line
	currentPiece = upAndComingPieces[0];

	//move up and coming pieces downwards
	for (int i = 0; i < gameModifiers.NumUpAndComingPieces - 1; i++)
		upAndComingPieces[i] = upAndComingPieces[i + 1];

	//fill last spot with new piece
	upAndComingPieces[gameModifiers.NumUpAndComingPieces - 1] = GetRandomPiece();

	Rectangle pieceBounds = currentPiece.GetBounds();
	currentPiecePosition = { gameModifiers.GridSize.x / 2 , (int)(-pieceBounds.y) };

	std::cout << "next piece in line" << std::endl;
}

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

	std::cout << "Placed piece!" << std::endl;

	NextPiece();
}

void SceneGame::HoldPiece()
{
	Piece tempPiece = holdingPiece;

	holdingPiece = currentPiece;

	//Go to next piece if we weren't holding a piece yet
	if (tempPiece.numBlocks == 0)
		NextPiece();
	//Swap held piece out into current piece
	else
		currentPiece = tempPiece;

	std::cout << "Hold piece" << std::endl;
}

#pragma endregion

#pragma region Grid

//Out of bounds cells do not count as empty and thus return false.
bool SceneGame::IsCellEmpty(int x, int y)
{
	if (x < 0 || x >= gameModifiers.GridSize.x || y < 0 || y >= gameModifiers.GridSize.y)
		return false;

	return grid[y][x] == EMPTY_BLOCK_COLOR;
}

void SceneGame::ClearLine(int line)
{
	//implement
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
	//Destroy grid

	for (int y = 0; y < gameModifiers.GridSize.y; y++)
		delete[] grid[y];

	delete[] grid;
}
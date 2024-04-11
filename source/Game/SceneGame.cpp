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
	float deltaTime = gameWindow.GetFrameTime();

	gravityPieceDeltaTime += deltaTime;
	timePlayingSeconds += deltaTime;

	//for testing purposes
	Vector2Int movement = { 0, 0 };

	//movement
	if (IsKeyPressed(KEY_RIGHT))
		movement.x = 1;
	else if (IsKeyPressed(KEY_LEFT))
		movement.x = -1;

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

	//for testing purposes
	if (IsKeyPressed(KEY_N))
		level++;
	else if (IsKeyPressed(KEY_B))
		level--;

	if (IsKeyPressed(KEY_SPACE))
	{
		HardDropPiece();
		PlacePiece();
	}
	else if (IsKeyPressed(KEY_C))
		HoldPiece();
	else
	{
		if (IsKeyPressed(KEY_DOWN))
			gravityPieceDeltaTime = 1.0f / 20.0f;

		int gravityLevel = std::min(level - 1, 14);
		float gravityMovementTime = IsKeyDown(KEY_DOWN) ? 1.0f / 20.0f : std::powf(0.8f - ((float)gravityLevel * 0.007f), (float)gravityLevel);
		
		while (gravityPieceDeltaTime >= gravityMovementTime)
		{
			gravityPieceDeltaTime -= gravityMovementTime;

			if (CanPieceExistAt({ currentPiecePosition.x, currentPiecePosition.y + 1 }))
				currentPiecePosition = { currentPiecePosition.x, currentPiecePosition.y + 1 };
			else
			{
				PlacePiece();
				break;
			}
		}
	}
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

	gravityPieceDeltaTime = 0.0f;

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

	Rectangle pieceBounds = currentPiece.GetBounds();
	currentPiecePosition = { gameModifiers.GridSize.x / 2 , (int)(-pieceBounds.y) };
	gravityPieceDeltaTime = 0.0f;

	std::cout << "Hold piece" << std::endl;
}

void SceneGame::HardDropPiece()
{
	//Instantly move piece downwards until it can't anymore
	while (CanPieceExistAt({ currentPiecePosition.x, currentPiecePosition.y + 1 }))
		currentPiecePosition = { currentPiecePosition.x, currentPiecePosition.y + 1 };
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
	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	float blockSize = std::min((float)(screenWidth / gameModifiers.GridSize.x), (float)(screenHeight / gameModifiers.GridSize.y));

	Vector2 totalGridSize = { blockSize * gameModifiers.GridSize.x, blockSize * gameModifiers.GridSize.y };
	float startGridPosX = ((float)screenWidth - totalGridSize.x) / 2.0f;

	//Draw grid
	for (int y = 0; y < gameModifiers.GridSize.y; y++)
	{
		for (int x = 0; x < gameModifiers.GridSize.x; x++)
		{
			if (grid[y][x] == EMPTY_BLOCK_COLOR)
				continue;

			raylib::Rectangle rect = { startGridPosX + blockSize * x, blockSize * y, blockSize, blockSize };
			rect.Draw(grid[y][x]);
		}
	}

	//Draw current piece
	for (int i = 0; i < currentPiece.numBlocks; i++)
	{
		raylib::Rectangle rect = { startGridPosX + blockSize * (currentPiecePosition.x + currentPiece.blockOffsets[i].x), blockSize * (currentPiecePosition.y + currentPiece.blockOffsets[i].y), blockSize, blockSize};
		rect.Draw(currentPiece.blockColors[i]);
	}

	raylib::Color borderColor = raylib::Color::Gray();
	borderColor.DrawLine({ startGridPosX, 0 }, { startGridPosX, (float)screenHeight }, 4.0f);
	borderColor.DrawLine({ startGridPosX + totalGridSize.x, 0 }, { startGridPosX + totalGridSize.x, (float)screenHeight }, 4.0f);

	//Draw score
	raylib::DrawText("Score: " + std::to_string(score), startGridPosX + totalGridSize.x + 10, 0, 36, raylib::Color::White());
	raylib::DrawText("Level: " + std::to_string(level), startGridPosX + totalGridSize.x + 10, 36, 36, raylib::Color::White());
	raylib::DrawText("Cleared: " + std::to_string(linesCleared), startGridPosX + totalGridSize.x + 10, 36 * 2, 36, raylib::Color::White());
	raylib::DrawText("Time: " + std::to_string(timePlayingSeconds), startGridPosX + totalGridSize.x + 10, 36 * 3, 36, raylib::Color::White());
}

void SceneGame::Destroy()
{
	//Destroy grid

	for (int y = 0; y < gameModifiers.GridSize.y; y++)
		delete[] grid[y];

	delete[] grid;
}
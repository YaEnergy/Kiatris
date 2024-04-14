#include "Game/SceneGame.h"
#include "Assets.h"

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

void SceneGame::StartGame(GameModifiers modifiers)
{
	if (grid != nullptr)
	{
		for (int y = 0; y < gameModifiers.GridSize.y; y++)
			delete[] grid[y];

		delete[] grid;
	}

	//main
	gameModifiers = modifiers;
	gameOver = false;

	//delta times
	gravityPieceDeltaTime = 0.0f;
	movementPieceDeltaTime = 0.0f;

	//statistics
	timePlayingSeconds = 0.0f;
	totalLinesCleared = 0;
	score = 0;
	level = 0;

	//pieces
	upAndComingPieces = std::vector<Piece>(modifiers.NumUpAndComingPieces);
	for (int i = 0; i < modifiers.NumUpAndComingPieces; i++)
		upAndComingPieces[i] = GetRandomPiece();

	NextPiece();

	//Create grid
	grid = new raylib::Color * [modifiers.GridSize.y];
	for (int y = 0; y < modifiers.GridSize.y; y++)
	{
		grid[y] = new raylib::Color[modifiers.GridSize.x];

		for (int x = 0; x < modifiers.GridSize.x; x++)
		{
			grid[y][x] = raylib::Color::Blank();
		}
	}
}

void SceneGame::UpdateGameplay()
{
	float deltaTime = gameWindow.GetFrameTime();

	gravityPieceDeltaTime += deltaTime;
	movementPieceDeltaTime += deltaTime;
	timePlayingSeconds += deltaTime;

	level = totalLinesCleared / 10;

	Piece piece = currentPiece;

	//rotation
	if (IsKeyPressed(KEY_E))
		piece = piece.GetLeftRotation();
	else if (IsKeyPressed(KEY_R))
		piece = piece.GetRightRotation();
	else if (IsKeyPressed(KEY_T))
		piece = piece.GetHalfCircleRotation();

	//movement
	Vector2Int movement = { 0, 0 };
	const float movePieceTime = 1.0f / 5.0f;

	if (IsKeyDown(KEY_RIGHT))
	{
		if (IsKeyPressed(KEY_RIGHT))
			movementPieceDeltaTime = movePieceTime;

		movement.x = 1;

	}
	else if (IsKeyDown(KEY_LEFT))
	{
		if (IsKeyPressed(KEY_LEFT))
			movementPieceDeltaTime = movePieceTime;

		movement.x = -1;
	}

	bool positionSuccess = false;

	if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)) && movementPieceDeltaTime >= movePieceTime)
	{
		while (movementPieceDeltaTime >= movePieceTime)
		{
			movementPieceDeltaTime -= movePieceTime;

			Vector2Int newPiecePosition = { currentPiecePosition.x + movement.x, currentPiecePosition.y + movement.y };

			if (CanPieceExistAt(piece, newPiecePosition))
			{
				currentPiecePosition = newPiecePosition;
				positionSuccess = true;
			}
			else
				break;
		}
	}
	else
		positionSuccess = CanPieceExistAt(piece, currentPiecePosition);

	//if new position is successful
	if (positionSuccess) //for rotations
		currentPiece = piece;

	if (IsKeyPressed(KEY_SPACE))
	{
		HardDropPiece();
		PlacePiece();

		if (!CanPieceExistAt(currentPiece, currentPiecePosition))
			EndGame();
	}
	else if (IsKeyPressed(KEY_C) && !hasSwitchedPiece)
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

			if (CanPieceExistAt(currentPiece, { currentPiecePosition.x, currentPiecePosition.y + 1 }))
				currentPiecePosition = { currentPiecePosition.x, currentPiecePosition.y + 1 };
			else
			{
				PlacePiece();
				break;
			}
		}

		if (!CanPieceExistAt(currentPiece, currentPiecePosition))
			EndGame();
	}

	//line clearing
	int linesCleared = 0;
	int scoreMultiplier = 0;

	//check all lines
	for (int y = 0; y < gameModifiers.GridSize.y; y++)
	{
		bool isClear = true;

		for (int x = 0; x < gameModifiers.GridSize.x; x++)
		{
			if (IsCellEmpty(x, y))
			{
				isClear = false;
				break;
			}
		}

		if (isClear)
		{
			linesCleared += 1;

			if (linesCleared == 1)
				scoreMultiplier = 1;
			else
				scoreMultiplier *= 4;

			ClearLine(y);
		}
	}

	if (linesCleared > 0)
		std::cout << "Cleared " + std::to_string(linesCleared) + " line(s) " << std::endl;

	totalLinesCleared += linesCleared;
	score += 1000 * scoreMultiplier;
}

void SceneGame::UpdateGameOver()
{
	if (IsKeyPressed(KEY_SPACE))
		StartGame(gameModifiers);
}

void SceneGame::EndGame()
{
	gameOver = true;
}

#pragma endregion

#pragma region Pieces

Piece SceneGame::GetRandomPiece()
{
	return Piece::GetMainPiece((MainPieceType)GetRandomValue(0, 6));
}

bool SceneGame::CanPieceExistAt(Piece piece, Vector2Int position)
{
	//if any block is out of bounds or not empty, then the piece can not exist at this position
	for (int i = 0; i < piece.numBlocks; i++)
	{
		if (!IsCellEmpty(position.x + piece.blockOffsets[i].x, position.y + piece.blockOffsets[i].y))
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

	currentPiecePosition = { gameModifiers.GridSize.x / 2 , 0 };

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

	hasSwitchedPiece = false;

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

	currentPiecePosition = { gameModifiers.GridSize.x / 2 , 0 };
	gravityPieceDeltaTime = 0.0f;
	hasSwitchedPiece = true;

	std::cout << "Hold piece" << std::endl;
}

void SceneGame::HardDropPiece()
{
	//Instantly move piece downwards until it can't anymore
	while (CanPieceExistAt(currentPiece, { currentPiecePosition.x, currentPiecePosition.y + 1 }))
		currentPiecePosition = { currentPiecePosition.x, currentPiecePosition.y + 1 };

	std::cout << "Hard drop piece" << std::endl;
}

#pragma endregion

#pragma region Grid

//Out of bounds cells do not count as empty and thus return false, unless this cell is above the grid but still within the left and right bounds.
bool SceneGame::IsCellEmpty(int x, int y)
{
	if (x < 0 || x >= gameModifiers.GridSize.x || y >= gameModifiers.GridSize.y)
		return false;

	if (y < 0)
		return true;

	return grid[y][x] == EMPTY_BLOCK_COLOR;
}

void SceneGame::ClearLine(int line)
{
	//shift everything downwards
	for (int y = line; y > 0; y--)
	{
		for (int x = 0; x < gameModifiers.GridSize.x; x++)
		{
			grid[y][x] = grid[y - 1][x];
		}
	}

	//clear line 0
	for (int x = 0; x < gameModifiers.GridSize.x; x++)
		grid[0][x] = EMPTY_BLOCK_COLOR;
	
	std::cout << "Cleared line " + std::to_string(line) << std::endl;
}

#pragma endregion


void SceneGame::Draw()
{
	//TODO: prototype, will improve soon

	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	float blockSize = std::min((float)(screenWidth / gameModifiers.GridSize.x), (float)(screenHeight / gameModifiers.GridSize.y));

	Vector2 totalGridSize = { blockSize * gameModifiers.GridSize.x, blockSize * gameModifiers.GridSize.y };
	float startGridPosX = ((float)screenWidth - totalGridSize.x) / 2.0f;

	//Block pieces
	raylib::Texture2D& blockTexture = GetTexture("BlockPiece");
	raylib::Rectangle blockTextureSource = { 0.0f, 0.0f, (float)blockTexture.width, (float)blockTexture.height };

	//Draw grid
	for (int y = 0; y < gameModifiers.GridSize.y; y++)
	{
		for (int x = 0; x < gameModifiers.GridSize.x; x++)
		{
			if (grid[y][x] == EMPTY_BLOCK_COLOR)
				continue;

			raylib::Rectangle rect = { startGridPosX + blockSize * x, blockSize * y, blockSize, blockSize };

			blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, grid[y][x]);
		}
	}

	if (!gameOver)
	{
		//Draw current piece
		for (int i = 0; i < currentPiece.numBlocks; i++)
		{
			raylib::Rectangle rect = { startGridPosX + blockSize * (currentPiecePosition.x + currentPiece.blockOffsets[i].x), blockSize * (currentPiecePosition.y + currentPiece.blockOffsets[i].y), blockSize, blockSize};

			blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, currentPiece.blockColors[i]);
		}

		if (gameModifiers.ShowPiecePreview)
		{
			//Draw current piece preview
			Vector2Int previewPosition = currentPiecePosition;

			//move preview position downwards until it hits the grid
			while (CanPieceExistAt(currentPiece, { previewPosition.x, previewPosition.y + 1 }))
			{
				previewPosition = { previewPosition.x, previewPosition.y + 1 };
			}

			for (int i = 0; i < currentPiece.numBlocks; i++)
			{
				raylib::Rectangle rect = { startGridPosX + blockSize * (previewPosition.x + currentPiece.blockOffsets[i].x), blockSize * (previewPosition.y + currentPiece.blockOffsets[i].y), blockSize, blockSize };

				blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, Fade(currentPiece.blockColors[i], 0.3f));
			}
		}
	}

	raylib::Color borderColor = raylib::Color::Gray();
	borderColor.DrawLine({ startGridPosX, 0 }, { startGridPosX, (float)screenHeight }, 4.0f);
	borderColor.DrawLine({ startGridPosX + totalGridSize.x, 0 }, { startGridPosX + totalGridSize.x, (float)screenHeight }, 4.0f);

	//held piece
	std::string holdText = "HELD";
	int holdTextFontSize = 36;
	int holdTextWidth = raylib::MeasureText(holdText, holdTextFontSize);
	raylib::DrawText(holdText, startGridPosX - holdTextWidth - 10, 0, holdTextFontSize, raylib::Color::White());

	raylib::Rectangle heldPieceBounds = holdingPiece.GetBounds();
	int holdPieceStartX = startGridPosX - holdTextWidth / 2.0f - 10;
	for (int i = 0; i < holdingPiece.numBlocks; i++)
	{
		raylib::Rectangle rect = { holdPieceStartX + blockSize * (holdingPiece.blockOffsets[i].x - (heldPieceBounds.x + heldPieceBounds.width) / 2.0f), holdTextFontSize + blockSize * (holdingPiece.blockOffsets[i].y - heldPieceBounds.y), blockSize, blockSize };

		blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, holdingPiece.blockColors[i]);
	}

	//up and coming pieces
	std::string nextText = "NEXT";
	int nextTextFontSize = 36;
	int nextTextWidth = raylib::MeasureText(nextText, nextTextFontSize);
	raylib::DrawText(nextText, startGridPosX - nextTextWidth - 10, holdTextFontSize + 4 * blockSize + 10, nextTextFontSize, raylib::Color::White());
	
	int nextPieceStartX = startGridPosX - nextTextWidth / 2.0f - 10;
	for (int pieceIndex = 0; pieceIndex < gameModifiers.NumUpAndComingPieces; pieceIndex++)
	{
		int pieceStartY = holdTextFontSize + nextTextFontSize + (4 * blockSize + 10) * (pieceIndex + 1);
		raylib::Rectangle pieceBounds = upAndComingPieces[pieceIndex].GetBounds();

		for (int i = 0; i < upAndComingPieces[pieceIndex].numBlocks; i++)
		{
			raylib::Rectangle rect = { holdPieceStartX + blockSize * (upAndComingPieces[pieceIndex].blockOffsets[i].x - (pieceBounds.x + pieceBounds.width) / 2.0f), pieceStartY + blockSize * (upAndComingPieces[pieceIndex].blockOffsets[i].y - pieceBounds.y * 1.5f), blockSize, blockSize };

			blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, upAndComingPieces[pieceIndex].blockColors[i]);
		}
	}

	//Draw score
	raylib::DrawText("Score: " + std::to_string(score), startGridPosX + totalGridSize.x + 10, 0, 36, raylib::Color::White());
	raylib::DrawText("Level: " + std::to_string(level), startGridPosX + totalGridSize.x + 10, 36, 36, raylib::Color::White());
	raylib::DrawText("Cleared: " + std::to_string(totalLinesCleared), startGridPosX + totalGridSize.x + 10, 36 * 2, 36, raylib::Color::White());
	raylib::DrawText("Time: " + std::to_string(timePlayingSeconds), startGridPosX + totalGridSize.x + 10, 36 * 3, 36, raylib::Color::White());
	raylib::DrawText("Switched piece: " + std::to_string(hasSwitchedPiece), startGridPosX + totalGridSize.x + 10, 36 * 4, 36, (hasSwitchedPiece ? raylib::Color::Red() : raylib::Color::White()));
}

void SceneGame::Destroy()
{
	//Destroy grid

	for (int y = 0; y < gameModifiers.GridSize.y; y++)
		delete[] grid[y];

	delete[] grid;
}
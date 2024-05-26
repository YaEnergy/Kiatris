#include "Kiatris.h"
#include "Game/SceneGame.h"
#include "Assets.h"

#include <format>

void SceneGame::Init()
{
	
}

void SceneGame::Update()
{
	switch (menuState)
	{
		case MENU_TITLE:
			UpdateTitleMenu();
			break;
		case MENU_OPTIONS:
			UpdateOptionsMenu();
			break;
		case MENU_CONTROLS:
			UpdateControlsMenu();
			break;
		default:

			if (gameOver)
			{
				UpdateGameOver();
				return;
			}

			raylib::Music& mainTheme = GetMusic("MainTheme");

			//Pausing and unpausing
			if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_F1))
			{
				gamePaused = !gamePaused;

				mainTheme.SetVolume(gamePaused ? 0.1f : 0.2f);
				GetSound("PlacePiece").Play();

				std::cout << "Paused state: " + std::to_string(gamePaused)  << std::endl;
			}

			if (!gamePaused)
				UpdateGameplay();

			if (!mainTheme.IsPlaying())
				mainTheme.Play();

			mainTheme.Update();

			break;
	}
}

#pragma region Gameplay

void SceneGame::SetGameModifiers(GameModifiers modifiers)
{
	//Delete old grid
	if (grid != nullptr)
	{
		for (int y = 0; y < gameModifiers.GridSize.y; y++)
			delete[] grid[y];

		delete[] grid;
	}

	gameModifiers = modifiers;

	//Create grid
	grid = new BlockCell * [modifiers.GridSize.y];
	for (int y = 0; y < modifiers.GridSize.y; y++)
	{
		grid[y] = new BlockCell[modifiers.GridSize.x];

		for (int x = 0; x < modifiers.GridSize.x; x++)
		{
			grid[y][x] = BlockCell(BLOCK_EMPTY, raylib::Color::Blank());
		}
	}

	upAndComingPieces.clear();
	upAndComingPieces.resize(modifiers.NumUpAndComingPieces);
}

void SceneGame::StartGame()
{
	
	//main
	gameOver = false;
	gamePaused = false;
	isClearingLines = false;
	clearingLines.clear();

	//delta times
	gravityPieceDeltaTime = 0.0f;
	movementPieceDeltaTime = 0.0f;
	deltaLineClearingTime = 0.0f;

	//statistics
	timePlayingSeconds = 0.0f;
	totalLinesCleared = 0;
	score = 0;
	level = 1;

	//pieces
	holdingPiece = Piece(0); //no piece
	for (int i = 0; i < gameModifiers.NumUpAndComingPieces; i++)
		upAndComingPieces[i] = GetRandomPieceFromBag();

	NextPiece();

	//Clear grid
	for (int y = 0; y < gameModifiers.GridSize.y; y++)
	{
		for (int x = 0; x < gameModifiers.GridSize.x; x++)
		{
			grid[y][x] = BlockCell(BLOCK_EMPTY, raylib::Color::Blank());
		}
	}

	//restart music
	raylib::Music& mainTheme = GetMusic("MainTheme");
	mainTheme.Seek(0.0f);
}

void SceneGame::UpdateGameplay()
{
	float deltaTime = gameWindow.GetFrameTime();

	timePlayingSeconds += deltaTime;

	if (isClearingLines)
	{
		deltaLineClearingTime += deltaTime;

		//Wait LINE_CLEAR_TIME_SECONDS for anim and then clear line
		if (deltaLineClearingTime >= lineClearTimeSeconds)
		{
			//Clear lines
			GetSound("LineClear").Play();
			int numClearedLines = clearingLines.size();

			for (int line : clearingLines)
				ClearLine(line);

			totalLinesCleared += numClearedLines;

			std::cout << "Cleared " + std::to_string(numClearedLines) + " line(s) " << std::endl;
			
			switch (numClearedLines)
			{
				//single
				case 1:
					score += 100 * level;
					break;
				//double
				case 2:
					score += 300 * level;
					break;
				//triple
				case 3:
					score += 500 * level;
					break;
				//tetris
				case 4:
					score += 800 * level;
					break;
			}

			isClearingLines = false;

			//Level up every total 10 lines cleared
			if (totalLinesCleared / 10 != (totalLinesCleared - numClearedLines) / 10)
			{
				GetSound("LevelUp").Play();
				level++;
			}

			clearingLines.clear();
		}
		else
			return;
	}

	gravityPieceDeltaTime += deltaTime;
	movementPieceDeltaTime += deltaTime;

	lineClearTimeSeconds = std::max(1.0f - 0.1f * level, 0.1f);

	UpdatePieceMovement();

	if (IsKeyPressed(KEY_SPACE))
	{
		HardDropPiece();
		PlacePiece();

		if (!CanPieceExistAt(currentPiece, currentPiecePosition))
			EndGame();
	}
	else if ((IsKeyPressed(KEY_C) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) && !hasSwitchedPiece)
		HoldPiece();
	else
		UpdatePieceGravity();
}

void SceneGame::UpdatePieceMovement()
{
	Piece piece = currentPiece;

	//rotation
	if (IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL) || IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_E))
		piece = piece.GetLeftRotation(); //Counter-clockwise
	else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_X) || IsKeyPressed(KEY_R))
		piece = piece.GetRightRotation(); //Clockwise
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
}

void SceneGame::UpdatePieceGravity()
{
	if (IsKeyPressed(KEY_DOWN))
		gravityPieceDeltaTime = 1.0f / 20.0f;

	int gravityLevel = std::min(level - 1, 14);
	float gravityMovementTime = IsKeyDown(KEY_DOWN) ? 1.0f / 20.0f : std::powf(0.8f - ((float)gravityLevel * 0.007f), (float)gravityLevel);

	while (gravityPieceDeltaTime >= gravityMovementTime)
	{
		gravityPieceDeltaTime -= gravityMovementTime;

		if (CanPieceExistAt(currentPiece, { currentPiecePosition.x, currentPiecePosition.y + 1 }))
		{
			currentPiecePosition = { currentPiecePosition.x, currentPiecePosition.y + 1 };

			//plus one point for each cell dropped with soft drop
			if (IsKeyDown(KEY_DOWN))
				score += 1;
		}
		else
		{
			PlacePiece();
			break;
		}
	}

	if (!CanPieceExistAt(currentPiece, currentPiecePosition))
		EndGame();
}

void SceneGame::LineClearCheck()
{
	//check all lines
	for (int y = 0; y < gameModifiers.GridSize.y; y++)
	{
		bool isClear = true;

		for (int x = 0; x < gameModifiers.GridSize.x; x++)
		{
			if (grid[y][x].state != BLOCK_GRID)
			{
				isClear = false;
				break;
			}
		}

		if (isClear)
		{
			//start line clear animation for blocks
			for (int x = 0; x < gameModifiers.GridSize.x; x++)
			{
				grid[y][x].state = BLOCK_CLEARING;
			}

			clearingLines.push_back(y);
			isClearingLines = true;
		}
	}

	if (isClearingLines)
	{
		deltaLineClearingTime = 0.0f;
		std::cout << "Clearing " + std::to_string(clearingLines.size()) + " line(s) " << std::endl;
	}
}

void SceneGame::UpdateGameOver()
{
	if (IsKeyPressed(KEY_DOWN))
		menuButtonIndex = Wrap(menuButtonIndex + 1, 0, 2);
	else if (IsKeyPressed(KEY_UP))
		menuButtonIndex = Wrap(menuButtonIndex - 1, 0, 2);

	switch (menuButtonIndex)
	{
		//retry button
		case 0:
			if (IsKeyPressed(KEY_SPACE))
				StartGame();

			break;
		//menu button
		case 1:
			if (IsKeyPressed(KEY_SPACE))
				ReturnToMenu();

			break;
	}
}

void SceneGame::EndGame()
{
	GetSound("GameOver").Play();
	gameOver = true;
	gamePaused = false;
	menuButtonIndex = 0;
	currentPiece = Piece();
}

void SceneGame::ReturnToMenu()
{
	gameOver = false;
	gamePaused = false;
	menuState = MENU_TITLE;
	menuButtonIndex = 0;
}

void SceneGame::DrawGame()
{
	const int UI_PIECE_LENGTH = 4;
	const int BASE_FONT_SIZE = 12;

	float windowTime = gameWindow.GetTime();

	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	raylib::Color gridBackgroundColor = raylib::Color::Black().Alpha(0.4f);

	raylib::Texture2D& blockTexture = GetTexture("BlockPiece");
	raylib::Rectangle blockTextureSource = { 0.0f, 0.0f, (float)blockTexture.width, (float)blockTexture.height };

	//Background
	blockTexture.Draw(raylib::Rectangle(Wrap(windowTime, 0.0f, 1.0f) * (float)blockTexture.width, Wrap(windowTime, 0.0f, 1.0f) * (float)blockTexture.height, (float)screenWidth / 1.5f, (float)screenHeight / 1.5f), { 0, 0, (float)screenWidth, (float)screenHeight }, { 0, 0 }, 0.0f, raylib::Color::DarkBlue());

	//Field

	float fieldXPadding = screenWidth / 10.0f;
	float fieldYPadding = screenHeight / 10.0f;

	float maxFieldWidth = screenWidth - fieldXPadding * 2;

	float maxFieldHeight = screenHeight - fieldYPadding * 2;

	//Grid size + Holding piece + Next pieces

	float blockSize = std::min(maxFieldWidth / (gameModifiers.GridSize.x + UI_PIECE_LENGTH * 2), maxFieldHeight / std::max(gameModifiers.GridSize.y, UI_PIECE_LENGTH * gameModifiers.NumUpAndComingPieces + gameModifiers.NumUpAndComingPieces));

	Vector2 fieldSize = { blockSize * (gameModifiers.GridSize.x + UI_PIECE_LENGTH * 2), blockSize * gameModifiers.GridSize.y };
	float fieldX = ((float)screenWidth - fieldSize.x) / 2.0f;

	Vector2 gridSize = { blockSize * gameModifiers.GridSize.x, blockSize * gameModifiers.GridSize.y };
	float gridX = ((float)screenWidth - gridSize.x) / 2.0f;

	//Grid background
	raylib::Rectangle(gridX, fieldYPadding, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

	DrawGrid(gridX, fieldYPadding, blockSize, blockTexture);

	//Draw pause overlay if paused
	if (gamePaused)
	{
		raylib::Rectangle(gridX, fieldYPadding, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

		std::string pausedText = "PAUSED";
		int pausedTextFontSize = (int)((float)BASE_FONT_SIZE / raylib::MeasureText(pausedText, BASE_FONT_SIZE) * gridSize.x / 2.0f);
		raylib::DrawText(pausedText, (int)(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f), (int)(fieldYPadding + gridSize.y / 2.0f - pausedTextFontSize / 2.0f), pausedTextFontSize, raylib::Color::White());
	}

	//Draw game over overlay if dead
	if (gameOver)
	{
		raylib::Rectangle(gridX, fieldYPadding, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

		std::string gameOverText = "GAME OVER";
		int gameOverTextFontSize = (int)((float)BASE_FONT_SIZE / raylib::MeasureText(gameOverText, BASE_FONT_SIZE) * gridSize.x / 1.5f);
		raylib::DrawText(gameOverText, (int)(gridX + gridSize.x / 2.0f - gridSize.x / 3.0f), (int)(fieldYPadding + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f), gameOverTextFontSize, raylib::Color::White());

		std::string retryText = "RETRY";
		int retryTextFontSize = (int)((float)BASE_FONT_SIZE / raylib::MeasureText(retryText, BASE_FONT_SIZE) * gridSize.x / 2.0f);
		raylib::DrawText(retryText, (int)(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f), (int)(fieldYPadding + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f + gameOverTextFontSize), retryTextFontSize, menuButtonIndex == 0 ? raylib::Color::Yellow() : raylib::Color::White());
	
		std::string menuText = "MENU";
		int menuTextFontSize = (int)((float)BASE_FONT_SIZE / raylib::MeasureText(menuText, BASE_FONT_SIZE) * gridSize.x / 2.0f);
		raylib::DrawText(menuText, (int)(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f), (int)(fieldYPadding + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f + gameOverTextFontSize + retryTextFontSize), menuTextFontSize, menuButtonIndex == 1 ? raylib::Color::Yellow() : raylib::Color::White());
	}

	//held piece
	std::string holdText = "HELD";
	int holdTextFontSize = (int)((float)BASE_FONT_SIZE / raylib::MeasureText(holdText, BASE_FONT_SIZE) * (UI_PIECE_LENGTH - 1.0f) * blockSize);
	int holdTextWidth = raylib::MeasureText(holdText, holdTextFontSize);

	float holdHeight = blockSize * UI_PIECE_LENGTH + holdTextFontSize;
	gridBackgroundColor.DrawRectangle({ fieldX, fieldYPadding, blockSize * UI_PIECE_LENGTH, holdHeight });

	raylib::DrawText(holdText, gridX - (UI_PIECE_LENGTH - 0.5f) * blockSize, fieldYPadding, holdTextFontSize, raylib::Color::White());

	float holdPieceStartX = gridX - 4 * blockSize;
	for (int i = 0; i < holdingPiece.numBlocks; i++)
	{
		raylib::Rectangle rect = { holdPieceStartX + blockSize * (holdingPiece.blockOffsets[i].x + 1), fieldYPadding + holdTextFontSize + blockSize * (holdingPiece.blockOffsets[i].y + 1), blockSize, blockSize };

		blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, holdingPiece.blockColors[i]);
	}

	//up and coming pieces
	std::string nextText = "NEXT";
	int nextTextFontSize = (int)((float)BASE_FONT_SIZE / raylib::MeasureText(nextText, BASE_FONT_SIZE) * (UI_PIECE_LENGTH - 1.0f) * blockSize);
	int nextTextWidth = raylib::MeasureText(nextText, nextTextFontSize);

	float nextHeight = blockSize * (UI_PIECE_LENGTH + 1) * gameModifiers.NumUpAndComingPieces - blockSize + nextTextFontSize;
	gridBackgroundColor.DrawRectangle({ gridX + gridSize.x, fieldYPadding, blockSize * UI_PIECE_LENGTH, nextHeight });

	raylib::DrawText(nextText, gridX + gridSize.x + 0.5f * blockSize, fieldYPadding, nextTextFontSize, raylib::Color::White());

	int nextPieceStartX = gridX + gridSize.x;
	for (int pieceIndex = 0; pieceIndex < gameModifiers.NumUpAndComingPieces; pieceIndex++)
	{
		int pieceStartY = fieldYPadding + nextTextFontSize + ((UI_PIECE_LENGTH + 1) * blockSize) * (pieceIndex);

		for (int i = 0; i < upAndComingPieces[pieceIndex].numBlocks; i++)
		{
			raylib::Rectangle rect = { nextPieceStartX + blockSize * (upAndComingPieces[pieceIndex].blockOffsets[i].x + 1), pieceStartY + blockSize * (upAndComingPieces[pieceIndex].blockOffsets[i].y + 1), blockSize, blockSize };

			blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, upAndComingPieces[pieceIndex].blockColors[i]);
		}
	}

	//Statistics
	int statPanelHeightPadding = 10;
	int statTextFontSize = (int)((float)BASE_FONT_SIZE / raylib::MeasureText("AAAAA", BASE_FONT_SIZE) * (UI_PIECE_LENGTH - 1.0f) * blockSize);

	gridBackgroundColor.DrawRectangle({ fieldX, fieldYPadding + holdHeight, blockSize * UI_PIECE_LENGTH, statTextFontSize * 8.0f + statPanelHeightPadding * 2.0f });
	//Score
	std::string scoreText = "SCORE";
	raylib::DrawText(scoreText, fieldX + 0.5f * blockSize, fieldYPadding + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statPanelHeightPadding, statTextFontSize, raylib::Color::White());

	std::string scoreValText = std::format("{:0>5}", score);
	raylib::DrawText(scoreValText, fieldX + 0.5f * blockSize, fieldYPadding + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize + statPanelHeightPadding, statTextFontSize, raylib::Color::White());

	//Level
	std::string levelText = "LEVEL";
	raylib::DrawText(levelText, fieldX + 0.5f * blockSize, fieldYPadding + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 2 + statPanelHeightPadding, statTextFontSize, raylib::Color::White());

	std::string levelValText = std::format("{:0>3}", level);
	raylib::DrawText(levelValText, fieldX + 0.5f * blockSize, fieldYPadding + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 3 + statPanelHeightPadding, statTextFontSize, raylib::Color::White());
	
	//Cleared lines
	std::string clearedText = "LINES";
	raylib::DrawText(clearedText, fieldX + 0.5f * blockSize, fieldYPadding + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 4 + statPanelHeightPadding, statTextFontSize, raylib::Color::White());

	std::string clearedValText = std::format("{:0>3}", totalLinesCleared);
	raylib::DrawText(clearedValText, fieldX + 0.5f * blockSize, fieldYPadding + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 5 + statPanelHeightPadding, statTextFontSize, raylib::Color::White());

	//Time
	std::string timeText = "TIME";
	raylib::DrawText(timeText, fieldX + 0.5f * blockSize, fieldYPadding + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 6 + statPanelHeightPadding, statTextFontSize, raylib::Color::White());

	std::string timeValText = std::format("{:0>3}", std::truncf(timePlayingSeconds));
	raylib::DrawText(timeValText, fieldX + 0.5f * blockSize, fieldYPadding + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 7 + statPanelHeightPadding, statTextFontSize, raylib::Color::White());

#ifdef DEBUG
	//Draw stats, temp
	raylib::DrawText("Switched piece: " + std::to_string(hasSwitchedPiece), 10, 24, 36, (hasSwitchedPiece ? raylib::Color::Red() : raylib::Color::White()));
#endif

	//TODO: replace some of these with rectangle outline draws
	//Borders
	raylib::Color borderColor = raylib::Color::SkyBlue();
	borderColor.DrawLine({ gridX, fieldYPadding }, { gridX, fieldYPadding + gridSize.y }, 4.0f);
	borderColor.DrawLine({ gridX + gridSize.x, fieldYPadding }, { gridX + gridSize.x, fieldYPadding + gridSize.y }, 4.0f);
	borderColor.DrawLine({ fieldX, fieldYPadding }, { fieldX + fieldSize.x, fieldYPadding }, 4.0f);
	borderColor.DrawLine({ gridX, fieldYPadding + gridSize.y }, { gridX + gridSize.x, fieldYPadding + gridSize.y }, 4.0f);
	
	borderColor.DrawLine({ fieldX, fieldYPadding}, { fieldX, fieldYPadding + holdHeight + statTextFontSize * 8 + statPanelHeightPadding * 2 }, 4.0f);
	borderColor.DrawLine({ fieldX, fieldYPadding + holdHeight }, { fieldX + UI_PIECE_LENGTH * blockSize, fieldYPadding + holdHeight }, 4.0f);
	borderColor.DrawLine({ fieldX, fieldYPadding + holdHeight + statTextFontSize * 8 + statPanelHeightPadding * 2 }, { fieldX + UI_PIECE_LENGTH * blockSize, fieldYPadding + holdHeight + statTextFontSize * 8 + statPanelHeightPadding * 2 }, 4.0f);

	borderColor.DrawLine({ fieldX + fieldSize.x, fieldYPadding }, { fieldX + fieldSize.x, fieldYPadding + nextHeight }, 4.0f);
	borderColor.DrawLine({ gridX + gridSize.x, fieldYPadding + nextHeight }, { fieldX + fieldSize.x, fieldYPadding + nextHeight }, 4.0f);
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

void SceneGame::RefillBag()
{
	bagPieces.clear();

	for (int i = 0; i < 7; i++)
		bagPieces.emplace_back(Piece::GetMainPiece((MainPieceType)i));

	std::cout << "Refilled bag" << std::endl;
}

Piece SceneGame::GetRandomPieceFromBag()
{
	//refill bag if empty
	if (bagPieces.size() == 0)
		RefillBag();

	int bagIndex = GetRandomValue(0, bagPieces.size() - 1);

	Piece piece = bagPieces[bagIndex];

	//remove piece from bag
	bagPieces.erase(std::next(bagPieces.begin(), bagIndex));

	return piece;
}

void SceneGame::NextPiece()
{
	//get next piece in line
	currentPiece = upAndComingPieces[0];

	//move up and coming pieces downwards
	for (int i = 0; i < gameModifiers.NumUpAndComingPieces - 1; i++)
		upAndComingPieces[i] = upAndComingPieces[i + 1];

	//fill last spot with random piece from bag
	upAndComingPieces[gameModifiers.NumUpAndComingPieces - 1] = GetRandomPieceFromBag();

	currentPiecePosition = { gameModifiers.GridSize.x / 2 , 0 };

	gravityPieceDeltaTime = 0.0f;

	std::cout << "next piece in line" << std::endl;
}

void SceneGame::PlacePiece()
{
	//Place piece into grid
	for (int i = 0; i < currentPiece.numBlocks; i++)
	{
		if (!IsCellInBounds(currentPiecePosition.x + currentPiece.blockOffsets[i].x, currentPiecePosition.y + currentPiece.blockOffsets[i].y))
			continue;

		grid[currentPiecePosition.y + currentPiece.blockOffsets[i].y][currentPiecePosition.x + currentPiece.blockOffsets[i].x].color = currentPiece.blockColors[i];
		grid[currentPiecePosition.y + currentPiece.blockOffsets[i].y][currentPiecePosition.x + currentPiece.blockOffsets[i].x].state = BLOCK_GRID;
	}

	hasSwitchedPiece = false;

	std::cout << "Placed piece!" << std::endl;

	//Checks for cleared lines
	//TODO: only check lines the placed piece affects instead of all lines
	LineClearCheck();

	GetSound("PlacePiece").Play();

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
	int cellsMoved = 0;

	//Instantly move piece downwards until it can't anymore
	while (CanPieceExistAt(currentPiece, { currentPiecePosition.x, currentPiecePosition.y + 1 }))
	{
		currentPiecePosition = { currentPiecePosition.x, currentPiecePosition.y + 1 };
		cellsMoved++;
	}

	score += cellsMoved * 2;

	std::cout << "Hard drop piece (" + std::to_string(cellsMoved) + " cells, +" + std::to_string(cellsMoved * 2) + " points)" << std::endl;
}

#pragma endregion

#pragma region Grid

bool SceneGame::IsCellInBounds(int x, int y) const
{
	return x >= 0 && x < gameModifiers.GridSize.x && y >= 0 && y < gameModifiers.GridSize.y;
}

//Out of bounds cells do not count as empty and thus return false, unless this cell is above the grid but still within the left and right bounds.
bool SceneGame::IsCellEmpty(int x, int y)
{
	if (x < 0 || x >= gameModifiers.GridSize.x || y >= gameModifiers.GridSize.y)
		return false;

	if (y < 0)
		return true;

	return grid[y][x].state == BLOCK_EMPTY;
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
		grid[0][x].state = BLOCK_EMPTY;
	
	std::cout << "Cleared line " + std::to_string(line) << std::endl;
}

void SceneGame::DrawGrid(float posX, float posY, float blockSize, raylib::Texture2D& blockTexture)
{
	raylib::Rectangle blockTextureSource = { 0.0f, 0.0f, (float)blockTexture.width, (float)blockTexture.height };

	//Draw grid cells
	for (int gridY = 0; gridY < gameModifiers.GridSize.y; gridY++)
	{
		for (int gridX = 0; gridX < gameModifiers.GridSize.x; gridX++)
		{
			if (grid[gridY][gridX].state == BLOCK_EMPTY)
				continue;

			raylib::Rectangle rect = { posX + blockSize * gridX, posY + blockSize * gridY, blockSize, blockSize };
			raylib::Color blockColor = grid[gridY][gridX].color;


			switch (grid[gridY][gridX].state)
			{
				case BLOCK_GRID:
					blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, blockColor);
					break;
				case BLOCK_CLEARING:
				{
					const int FLASH_LENGTH = 6;

					float startFlashTime = (lineClearTimeSeconds - (lineClearTimeSeconds / gameModifiers.GridSize.x) * (FLASH_LENGTH - 2)) / gameModifiers.GridSize.x * (gridX);
					float endFlashTime = (lineClearTimeSeconds - (lineClearTimeSeconds / gameModifiers.GridSize.x) * (FLASH_LENGTH - 2)) / gameModifiers.GridSize.x * (gridX + FLASH_LENGTH);

					if (deltaLineClearingTime < endFlashTime)
						blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, deltaLineClearingTime >= startFlashTime ? raylib::Color::White() : blockColor);
					
					break;
				}
				default:
					break;
			}
		}
	}

	if (!gameOver && !isClearingLines)
	{
		//Draw current piece
		for (int i = 0; i < currentPiece.numBlocks; i++)
		{
			if (!IsCellInBounds(currentPiecePosition.x + currentPiece.blockOffsets[i].x, currentPiecePosition.y + currentPiece.blockOffsets[i].y))
				continue;

			raylib::Rectangle rect = { posX + blockSize * (currentPiecePosition.x + currentPiece.blockOffsets[i].x), posY + blockSize * (currentPiecePosition.y + currentPiece.blockOffsets[i].y), blockSize, blockSize };

			blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, currentPiece.blockColors[i]);
		}

		if (gameModifiers.ShowPiecePreview && currentPiece.numBlocks != 0)
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
				if (!IsCellInBounds(previewPosition.x + currentPiece.blockOffsets[i].x, previewPosition.y + currentPiece.blockOffsets[i].y))
					continue;

				raylib::Rectangle rect = { posX + blockSize * (previewPosition.x + currentPiece.blockOffsets[i].x), posY + blockSize * (previewPosition.y + currentPiece.blockOffsets[i].y), blockSize, blockSize };

				blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, Fade(currentPiece.blockColors[i], 0.3f));
			}
		}
	}
}

#pragma endregion

#pragma region Menus
void SceneGame::UpdateTitleMenu()
{
	if (IsKeyPressed(KEY_DOWN))
		menuButtonIndex = Wrap(menuButtonIndex + 1, 0, 4);
	else if (IsKeyPressed(KEY_UP))
		menuButtonIndex = Wrap(menuButtonIndex - 1, 0, 4);

	//Press buttons
	if (menuButtonIndex == BUTTON_START_INDEX && IsKeyPressed(KEY_SPACE))
	{
		menuState = MENU_NONE;
		StartGame();
	}
	else if (menuButtonIndex == BUTTON_OPTIONS_INDEX && IsKeyPressed(KEY_SPACE))
	{
		menuState = MENU_OPTIONS;
	}
	else if (menuButtonIndex == BUTTON_CONTROLS_INDEX && IsKeyPressed(KEY_SPACE))
	{
		menuState = MENU_CONTROLS;
	}
	else if (menuButtonIndex == BUTTON_QUIT_INDEX && IsKeyPressed(KEY_SPACE))
	{
		//TODO: currently actually crashes the game, fix
		gameWindow.Close();
	}
}

void SceneGame::UpdateOptionsMenu()
{

}

void SceneGame::UpdateControlsMenu()
{

}

void SceneGame::DrawTitleMenu() 
{
	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	//Background
	raylib::Color backgroundColor = raylib::Color(30, 30, 30, 120);
	backgroundColor.DrawRectangle(0, 0, screenWidth, screenHeight);

	//Icon
	raylib::Texture2D& iconTexture = GetTexture("Icon");
	raylib::Rectangle iconSourceRect = raylib::Rectangle{ 0, 0, (float)iconTexture.width, (float)iconTexture.height };
	iconTexture.Draw(iconSourceRect, raylib::Rectangle{ screenWidth / 2.0f, screenHeight / 2.0f, (float)iconTexture.width, (float)iconTexture.height }, { (float)iconTexture.width / 2.0f, (float)iconTexture.height / 2.0f }, 0.0f, raylib::Color::White());

	//Buttons

	//For debug purposes
	DrawText(TextFormat("Button index: %i", menuButtonIndex), 12, 12 + 24, 24, raylib::Color::White());
}

void SceneGame::DrawOptionsMenu()
{

}

void SceneGame::DrawControlsMenu()
{

}
#pragma endregion


void SceneGame::Draw()
{
	DrawGame();

	switch (menuState)
	{
		case MENU_TITLE:
			DrawTitleMenu();
			break;
		case MENU_OPTIONS:
			DrawOptionsMenu();
			break;
		case MENU_CONTROLS:
			DrawControlsMenu();
			break;
		default:
			break;
	}
}

void SceneGame::Destroy()
{
	//Destroy grid

	for (int y = 0; y < gameModifiers.GridSize.y; y++)
		delete[] grid[y];

	delete[] grid;
}
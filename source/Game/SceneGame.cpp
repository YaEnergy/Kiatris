#include "Kiatris.h"
#include "Game/SceneGame.h"
#include "Assets.h"

#include <format>

const float BASE_FONT_SIZE = 12.0f;

//Base font spacing multiplier
const float BASE_FONT_SPACING = 0.1f;

/// Returns a text size that fits the given text within the specified width using the specified font and spacing (1.0 = letter height)
static float FitTextWidth(raylib::Font& font, const std::string text, const float width, const float percentageSpacing)
{
	return BASE_FONT_SIZE / font.MeasureText(text, BASE_FONT_SIZE, BASE_FONT_SIZE * percentageSpacing).x * width;
}

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

			movementPieceDeltaTime = 0.0f;
			gravityPieceDeltaTime = 0.0f;
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
	const float BASE_FONT_SIZE = 12.0f;

	float windowTime = gameWindow.GetTime();

	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	raylib::Font& mainFont = GetFont("MainFont");

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
	float fieldY = ((float)screenHeight - fieldSize.y) / 2.0f;

	Vector2 gridSize = { blockSize * gameModifiers.GridSize.x, blockSize * gameModifiers.GridSize.y };
	float gridX = ((float)screenWidth - gridSize.x) / 2.0f;

	//Grid background
	raylib::Rectangle(gridX, fieldY, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

	DrawGrid(gridX, fieldY, blockSize, blockTexture);

	//Draw pause overlay if paused
	if (gamePaused)
	{
		raylib::Rectangle(gridX, fieldY, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

		std::string pausedText = "PAUSED";
		float pausedTextFontSize = FitTextWidth(mainFont, pausedText, gridSize.x / 2.0f, BASE_FONT_SPACING);

		if (!showStrobingLights || Wrap(gameWindow.GetTime(), 0.0f, 1.0f) <= 0.5f)
			mainFont.DrawText(pausedText, raylib::Vector2(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f, fieldY + gridSize.y / 2.0f - pausedTextFontSize / 2.0f), pausedTextFontSize, pausedTextFontSize * BASE_FONT_SPACING, raylib::Color::White());
	}

	//Draw game over overlay if dead
	if (gameOver)
	{
		raylib::Rectangle(gridX, fieldY, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

		std::string gameOverText = "GAME OVER";
		float gameOverTextFontSize = FitTextWidth(mainFont, gameOverText, gridSize.x / 1.5f, BASE_FONT_SPACING);

		if (!showStrobingLights || Wrap(gameWindow.GetTime(), 0.0f, 1.0f) > 0.5f)
			mainFont.DrawText(gameOverText, raylib::Vector2(gridX + gridSize.x / 2.0f - gridSize.x / 3.0f, fieldY + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f), gameOverTextFontSize, gameOverTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

		std::string retryText = "RETRY";
		float retryTextFontSize = FitTextWidth(mainFont, retryText, gridSize.x / 2.0f, BASE_FONT_SPACING);
		mainFont.DrawText(retryText, raylib::Vector2(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f, fieldY + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f + gameOverTextFontSize), retryTextFontSize, retryTextFontSize * BASE_FONT_SPACING, menuButtonIndex == 0 ? raylib::Color::Yellow() : raylib::Color::White());
	
		std::string menuText = "MENU";
		int menuTextFontSize = FitTextWidth(mainFont, menuText, gridSize.x / 2.0f, BASE_FONT_SPACING);
		mainFont.DrawText(menuText, raylib::Vector2(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f, fieldY + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f + gameOverTextFontSize + retryTextFontSize), menuTextFontSize, menuTextFontSize * BASE_FONT_SPACING, menuButtonIndex == 1 ? raylib::Color::Yellow() : raylib::Color::White());
	}

	//held piece
	std::string holdText = "HELD";
	float holdTextFontSize = FitTextWidth(mainFont, holdText, (UI_PIECE_LENGTH - 1.0f) * blockSize, BASE_FONT_SPACING);
	float holdTextWidth = mainFont.MeasureText(holdText, holdTextFontSize, holdTextFontSize / 10.0f).x;

	float holdHeight = blockSize * UI_PIECE_LENGTH + holdTextFontSize;
	gridBackgroundColor.DrawRectangle({ fieldX, fieldY, blockSize * UI_PIECE_LENGTH, holdHeight });

	mainFont.DrawText(holdText, raylib::Vector2(gridX - (UI_PIECE_LENGTH - 0.5f) * blockSize, fieldY), holdTextFontSize, holdTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	float holdPieceStartX = gridX - 4 * blockSize;
	for (int i = 0; i < holdingPiece.numBlocks; i++)
	{
		raylib::Rectangle rect = { holdPieceStartX + blockSize * (holdingPiece.blockOffsets[i].x + 1), fieldY + holdTextFontSize + blockSize * (holdingPiece.blockOffsets[i].y + 1), blockSize, blockSize };

		blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, holdingPiece.blockColors[i]);
	}

	//up and coming pieces
	std::string nextText = "NEXT";
	float nextTextFontSize = FitTextWidth(mainFont, nextText, (UI_PIECE_LENGTH - 1.0f) * blockSize, BASE_FONT_SPACING);
	float nextTextWidth = mainFont.MeasureText(nextText, nextTextFontSize, nextTextFontSize * BASE_FONT_SPACING).x;

	float nextHeight = blockSize * (UI_PIECE_LENGTH + 1) * gameModifiers.NumUpAndComingPieces - blockSize + nextTextFontSize;
	gridBackgroundColor.DrawRectangle({ gridX + gridSize.x, fieldY, blockSize * UI_PIECE_LENGTH, nextHeight });

	mainFont.DrawText(nextText, raylib::Vector2(gridX + gridSize.x + 0.5f * blockSize, fieldY), nextTextFontSize, nextTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	int nextPieceStartX = gridX + gridSize.x;
	for (int pieceIndex = 0; pieceIndex < gameModifiers.NumUpAndComingPieces; pieceIndex++)
	{
		int pieceStartY = fieldY + nextTextFontSize + ((UI_PIECE_LENGTH + 1) * blockSize) * (pieceIndex);

		for (int i = 0; i < upAndComingPieces[pieceIndex].numBlocks; i++)
		{
			raylib::Rectangle rect = { nextPieceStartX + blockSize * (upAndComingPieces[pieceIndex].blockOffsets[i].x + 1), pieceStartY + blockSize * (upAndComingPieces[pieceIndex].blockOffsets[i].y + 1), blockSize, blockSize };

			blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, upAndComingPieces[pieceIndex].blockColors[i]);
		}
	}

	//Statistics
	int statPanelHeightPadding = 10;
	float statTextFontSize = FitTextWidth(mainFont, "AAAAA", (UI_PIECE_LENGTH - 1.0f) * blockSize, BASE_FONT_SPACING);

	gridBackgroundColor.DrawRectangle({ fieldX, fieldY + holdHeight, blockSize * UI_PIECE_LENGTH, statTextFontSize * 8.0f + statPanelHeightPadding * 2.0f });
	
	//Score
	std::string scoreText = "SCORE";
	mainFont.DrawText(scoreText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	std::string scoreValText = std::format("{:0>6}", score);
	mainFont.DrawText(scoreValText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	//Level
	std::string levelText = "LEVEL";
	mainFont.DrawText(levelText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 2 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	std::string levelValText = std::format("{:0>3}", level);
	mainFont.DrawText(levelValText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 3 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());
	
	//Cleared lines
	std::string clearedText = "LINES";
	mainFont.DrawText(clearedText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 4 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	std::string clearedValText = std::format("{:0>3}", totalLinesCleared);
	mainFont.DrawText(clearedValText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 5 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	//Time
	std::string timeText = "TIME";
	mainFont.DrawText(timeText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 6 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	std::string timeValText = std::format("{:0>3}", std::truncf(timePlayingSeconds));
	mainFont.DrawText(timeValText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 7 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

	//Borders
	raylib::Color borderColor = raylib::Color::SkyBlue();
	
	if (gameOver)
		borderColor = (Wrap(gameWindow.GetTime(), 0.0f, 1.0f) <= 0.5f || !showStrobingLights) ? raylib::Color::Red() : borderColor;
	else if (isClearingLines)
		borderColor = raylib::Color::Yellow();

	float borderThickness = 6.0f * std::min((float)fieldSize.x / DESIGN_WIDTH, (float)fieldSize.y / DESIGN_HEIGHT);

	borderColor.DrawLine({ gridX, fieldY }, { gridX, fieldY + gridSize.y }, borderThickness);
	borderColor.DrawLine({ gridX + gridSize.x, fieldY }, { gridX + gridSize.x, fieldY + gridSize.y }, borderThickness);
	borderColor.DrawLine({ gridX - borderThickness / 2.0f, fieldY }, { fieldX + fieldSize.x + borderThickness / 2.0f, fieldY }, borderThickness);
	borderColor.DrawLine({ gridX - borderThickness / 2.0f, fieldY + gridSize.y }, { gridX + gridSize.x + borderThickness / 2.0f, fieldY + gridSize.y }, borderThickness);
	
	raylib::Color holdPieceBorderColor = hasSwitchedPiece ? raylib::Color::Red() : borderColor;
	holdPieceBorderColor.DrawLine({ fieldX - borderThickness / 2.0f, fieldY }, { fieldX + UI_PIECE_LENGTH * blockSize - borderThickness / 2.0f, fieldY }, borderThickness);
	holdPieceBorderColor.DrawLine({ fieldX, fieldY }, { fieldX, fieldY + holdHeight }, borderThickness);
	holdPieceBorderColor.DrawLine({ fieldX - borderThickness / 2.0f, fieldY + holdHeight }, { fieldX + UI_PIECE_LENGTH * blockSize - borderThickness / 2.0f, fieldY + holdHeight }, borderThickness);

	borderColor.DrawLine({ fieldX, fieldY + holdHeight}, { fieldX, fieldY + holdHeight + statTextFontSize * 8 + statPanelHeightPadding * 2 }, borderThickness);
	borderColor.DrawLine({ fieldX - borderThickness / 2.0f, fieldY + holdHeight + statTextFontSize * 8 + statPanelHeightPadding * 2 }, { fieldX - borderThickness / 2.0f + UI_PIECE_LENGTH * blockSize, fieldY + holdHeight + statTextFontSize * 8 + statPanelHeightPadding * 2 }, borderThickness);

	borderColor.DrawLine({ fieldX + fieldSize.x, fieldY }, { fieldX + fieldSize.x, fieldY + nextHeight }, borderThickness);
	borderColor.DrawLine({ gridX + gridSize.x, fieldY + nextHeight }, { fieldX + fieldSize.x + borderThickness / 2.0f, fieldY + nextHeight }, borderThickness);
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

	switch (menuButtonIndex)
	{
		//start button
		case 0:
			if (IsKeyPressed(KEY_SPACE))
			{
				menuState = MENU_NONE;
				StartGame();
			}
			break;
		//options button
		case 1:
			if (IsKeyPressed(KEY_SPACE))
			{
				menuState = MENU_OPTIONS;
			}
			break;
		//controls button
		case 2:
			if (IsKeyPressed(KEY_SPACE))
			{
				menuState = MENU_CONTROLS;
			}
			break;
		//quit button
		case 3:
			if (IsKeyPressed(KEY_SPACE))
			{
				//TODO: currently actually crashes the game, fix
				gameWindow.Close();
			}
			break;
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

	float aspectScale = std::min((float)screenWidth / DESIGN_WIDTH, (float)screenHeight / DESIGN_HEIGHT);

	raylib::Font& mainFont = GetFont("MainFont");

	//Background
	raylib::Color backgroundColor = raylib::Color::Black().Alpha(0.4f);
	backgroundColor.DrawRectangle(0, 0, screenWidth, screenHeight);

	//Icon
	float iconScale = 1.0f * aspectScale;
	raylib::Texture2D& iconTexture = GetTexture("Icon");
	raylib::Rectangle iconSourceRect = raylib::Rectangle{ 0, 0, (float)iconTexture.width, (float)iconTexture.height };
	iconTexture.Draw(iconSourceRect, raylib::Rectangle{ screenWidth / 2.0f, screenHeight / 2.0f - (float)iconTexture.height * iconScale, (float)iconTexture.width * iconScale, (float)iconTexture.height * iconScale }, { (float)iconTexture.width / 2.0f * iconScale, (float)iconTexture.height / 2.0f * iconScale }, 0.0f, raylib::Color::White());

	//Title
	std::string titleText = "KIATRIS";
	float titleTextSize = 80 * aspectScale;
	float titleWidth = mainFont.MeasureText(titleText, titleTextSize, titleTextSize * BASE_FONT_SPACING).x;
	float titleTextX = (screenWidth - titleWidth) / 2.0f;

	for (int i = 0; i < titleText.length(); i++)
	{
		std::string titleChar = std::string(1, titleText.at(i));

		mainFont.DrawText(std::string(1, titleText.at(i)), raylib::Vector2(titleTextX, screenHeight / 2.0f - (float)iconTexture.height * iconScale * 1.5f - titleTextSize + sinf(gameWindow.GetTime() * 3.0f + i) * 4.0f * aspectScale), titleTextSize, titleTextSize * BASE_FONT_SPACING, raylib::Color::FromHSV(Wrap(gameWindow.GetTime() * 120.0f + 30.0f * i, 0.0f, 360.0f), 1.0f, 1.0f));
		
		int titleCharWidth = raylib::MeasureText(titleChar, titleTextSize);
		titleTextX += titleCharWidth + (titleTextSize / 10);
	}

	//Buttons
	float buttonTextSize = 64 * aspectScale;

	std::string startText = "START";
	float startWidth = mainFont.MeasureText(startText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(startText, raylib::Vector2(screenWidth / 2.0f - startWidth / 2.0f, screenHeight / 2.0f - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 0 ? raylib::Color::Yellow() : raylib::Color::White());

	std::string optionsText = "OPTIONS";
	float optionsWidth = mainFont.MeasureText(optionsText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(optionsText, raylib::Vector2(screenWidth / 2.0f - optionsWidth / 2.0f, screenHeight / 2.0f + buttonTextSize - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 1 ? raylib::Color::Yellow() : raylib::Color::White());

	std::string controlsText = "CONTROLS";
	float controlsWidth = mainFont.MeasureText(controlsText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(controlsText, raylib::Vector2(screenWidth / 2.0f - optionsWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 2 - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 2 ? raylib::Color::Yellow() : raylib::Color::White());

	std::string quitText = "QUIT";
	float quitWidth = mainFont.MeasureText(quitText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(quitText, raylib::Vector2(screenWidth / 2.0f - quitWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 3 - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 3 ? raylib::Color::Yellow() : raylib::Color::White());
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

	//For debug purposes
	DrawText(TextFormat("Button index: %i", menuButtonIndex), 12, 12 + 24, 24, raylib::Color::White());
}

void SceneGame::Destroy()
{
	//Destroy grid

	for (int y = 0; y < gameModifiers.GridSize.y; y++)
		delete[] grid[y];

	delete[] grid;
}
#include "Kiatris.h"
#include "Game/SceneGame.h"
#include "Assets.h"

const float BASE_FONT_SIZE = 12.0f;

//Base font spacing multiplier
const float BASE_FONT_SPACING = 0.1f;

/// Returns a text size that fits the given text within the specified width using the specified font and spacing (1.0 = letter height)
static float FitTextWidth(raylib::Font& font, const std::string text, const float width, const float percentageSpacing)
{
	return BASE_FONT_SIZE / font.MeasureText(text, BASE_FONT_SIZE, BASE_FONT_SIZE * percentageSpacing).x * width;
}

static bool IsConfirmButtonPressed()
{
	return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
}

void SceneGame::Init()
{
	
}

void SceneGame::Update()
{
	//Menu theme
	if (menuState != MENU_NONE && gameOptions.PlayMusic)
	{
		raylib::Music& menuTheme = GetMusic("MenuTheme");

		if (!menuTheme.IsPlaying())
			menuTheme.Play();

		menuTheme.Update();
	}

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
		case MENU_CREDITS:
			UpdateCreditsMenu();
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

#if DEBUG
			if (IsKeyPressed(KEY_B))
				level++;
#endif

			if (!gamePaused)
				UpdateGameplay();

			if (!mainTheme.IsPlaying())
				mainTheme.Play();

			if (gameOptions.PlayMusic)
				mainTheme.Update();

			break;
	}
}

#pragma region Gameplay

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

	upAndComingPieces.clear();
	upAndComingPieces.resize(gameOptions.NumUpAndComingPieces);

	for (int i = 0; i < gameOptions.NumUpAndComingPieces; i++)
		upAndComingPieces[i] = GetRandomPieceFromBag();

	NextPiece();

	//Clear grid
	for (int y = 0; y < gameOptions.GridSize.y; y++)
	{
		for (int x = 0; x < gameOptions.GridSize.x; x++)
		{
			grid[y][x] = BlockCell(BLOCK_EMPTY, raylib::Color::Blank());
		}
	}

	//restart main theme
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
			int numClearedLines = (int)clearingLines.size();

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

	UpdatePieceRotation();
	UpdatePieceMovement();

	if (IsConfirmButtonPressed())
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

void SceneGame::LineClearCheck(int topY, int bottomY)
{
	//check affected lines
	for (int y = topY; y <= bottomY; y++)
	{
		bool isClear = true;

		for (int x = 0; x < gameOptions.GridSize.x; x++)
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
			for (int x = 0; x < gameOptions.GridSize.x; x++)
			{
				grid[y][x].state = BLOCK_CLEARING;
			}

			clearingLines.push_back(y);
			isClearingLines = true;
		}
	}

	std::cout << "Checked lines " + std::to_string(topY) + " - " + std::to_string(bottomY) << std::endl;

	if (isClearingLines)
	{
		deltaLineClearingTime = 0.0f;
		std::cout << "Clearing " + std::to_string(clearingLines.size()) + " line(s) " << std::endl;
	}
}

void SceneGame::UpdateGameOver()
{
	UpdateMenuButtonNagivation(0, 1);

	switch (menuButtonIndex)
	{
		//retry button
		case 0:
			if (IsConfirmButtonPressed())
				StartGame();

			break;
		//menu button
		case 1:
			if (IsConfirmButtonPressed())
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

	//restart menu theme
	raylib::Music& menuTheme = GetMusic("MenuTheme");
	menuTheme.Seek(0.0f);
}

void SceneGame::DrawGame()
{
	const int UI_PIECE_LENGTH = 4;
	const float BASE_FONT_SIZE = 12.0f;

	double windowTime = gameWindow.GetTime();

	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	raylib::Font& mainFont = GetFont("MainFont");

	raylib::Color mainColor = raylib::Color::FromHSV(45.0f * (level - 1) + sinf((float)gameWindow.GetTime()) * 5.0f + 211.0f, 1.0f, 0.8f);

	raylib::Color gridBackgroundColor = raylib::Color::Black().Alpha(0.6f);

	raylib::Texture2D& blockTexture = GetTexture("BlockPiece");
	raylib::Rectangle blockTextureSource = { 0.0f, 0.0f, (float)blockTexture.width, (float)blockTexture.height };

	raylib::Color borderColor = raylib::Color::FromHSV(45.0f * (level - 1) - sinf((float)gameWindow.GetTime()) * 5.0f + 221.0f, 1.0f, 1.0f);

	if (gameOver)
		borderColor = (Wrap((float)gameWindow.GetTime(), 0.0f, 0.5f) < 0.25f || !gameOptions.EnableStrobingLights) ? raylib::Color::Red() : borderColor;
	else if (isClearingLines)
		borderColor = raylib::Color(255 - borderColor.r, 255 - borderColor.g, 255 - borderColor.b, borderColor.a);

	//Background
	blockTexture.Draw(raylib::Rectangle(Wrap((float)windowTime, 0.0f, 1.0f) * (float)blockTexture.width, Wrap((float)windowTime, 0.0f, 1.0f) * (float)blockTexture.height, (float)screenWidth / 1.5f, (float)screenHeight / 1.5f), { 0, 0, (float)screenWidth, (float)screenHeight }, { 0, 0 }, 0.0f, mainColor.Brightness(0.2f));

	//Field

	float fieldXPadding = screenWidth / 10.0f;
	float fieldYPadding = screenHeight / 10.0f;

	float maxFieldWidth = screenWidth - fieldXPadding * 2;

	float maxFieldHeight = screenHeight - fieldYPadding * 2;

	//Grid size + Holding piece + Next pieces

	float blockSize = std::min(maxFieldWidth / (gameOptions.GridSize.x + UI_PIECE_LENGTH * 2), maxFieldHeight / std::max(gameOptions.GridSize.y, UI_PIECE_LENGTH * gameOptions.NumUpAndComingPieces + gameOptions.NumUpAndComingPieces));

	Vector2 fieldSize = { blockSize * (gameOptions.GridSize.x + UI_PIECE_LENGTH * 2), blockSize * gameOptions.GridSize.y };
	float fieldX = ((float)screenWidth - fieldSize.x) / 2.0f;
	float fieldY = ((float)screenHeight - fieldSize.y) / 2.0f;

	Vector2 gridSize = { blockSize * gameOptions.GridSize.x, blockSize * gameOptions.GridSize.y };
	float gridX = ((float)screenWidth - gridSize.x) / 2.0f;

	float aspectScale = std::min((float)fieldSize.x / DESIGN_WIDTH, (float)fieldSize.y / DESIGN_HEIGHT);

	//Drawing grid
	{
		//Grid background
		raylib::Rectangle(gridX, fieldY, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

		//Horizontal grid lines
		for (int y = 1; y < gameOptions.GridSize.y; y++)
		{
			borderColor.Alpha(0.2f).DrawLine(Vector2{gridX, fieldY + y * blockSize}, Vector2{gridX + gridSize.x, fieldY + y * blockSize}, (int)(3.0f * aspectScale));
		}

		//Vertical grid lines
		for (int x = 1; x < gameOptions.GridSize.x; x++)
		{
			borderColor.Alpha(0.2f).DrawLine(Vector2{gridX + x * blockSize, fieldY}, Vector2{gridX + x * blockSize, fieldY + gridSize.y}, (int)(3.0f * aspectScale));
		}

		DrawGrid(gridX, fieldY, blockSize, blockTexture);
	}

	//Draw pause overlay if paused
	if (gamePaused)
	{
		raylib::Rectangle(gridX, fieldY, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

		std::string pausedText = "PAUSED";
		float pausedTextFontSize = FitTextWidth(mainFont, pausedText, gridSize.x / 2.0f, BASE_FONT_SPACING);

		if (!gameOptions.EnableStrobingLights || Wrap((float)gameWindow.GetTime(), 0.0f, 0.5f) < 0.25f)
			mainFont.DrawText(pausedText, raylib::Vector2(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f, fieldY + gridSize.y / 2.0f - pausedTextFontSize / 2.0f), pausedTextFontSize, pausedTextFontSize * BASE_FONT_SPACING, raylib::Color::White());
	}

	//Draw game over overlay if dead
	if (gameOver)
	{
		raylib::Rectangle(gridX, fieldY, gridSize.x, gridSize.y).Draw(gridBackgroundColor);

		std::string gameOverText = "GAME OVER";
		float gameOverTextFontSize = FitTextWidth(mainFont, gameOverText, gridSize.x / 1.5f, BASE_FONT_SPACING);

		if (!gameOptions.EnableStrobingLights || Wrap((float)gameWindow.GetTime(), 0.0f, 0.5f) < 0.25f)
			mainFont.DrawText(gameOverText, raylib::Vector2(gridX + gridSize.x / 2.0f - gridSize.x / 3.0f, fieldY + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f), gameOverTextFontSize, gameOverTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

		std::string retryText = "RETRY";
		float retryTextFontSize = FitTextWidth(mainFont, retryText, gridSize.x / 2.0f, BASE_FONT_SPACING);
		mainFont.DrawText(retryText, raylib::Vector2(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f, fieldY + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f + gameOverTextFontSize), retryTextFontSize, retryTextFontSize * BASE_FONT_SPACING, menuButtonIndex == 0 ? raylib::Color::Yellow() : raylib::Color::White());
	
		std::string menuText = "MENU";
		float menuTextFontSize = FitTextWidth(mainFont, menuText, gridSize.x / 2.0f, BASE_FONT_SPACING);
		mainFont.DrawText(menuText, raylib::Vector2(gridX + gridSize.x / 2.0f - gridSize.x / 4.0f, fieldY + gridSize.y / 2.0f - gameOverTextFontSize / 2.0f + gameOverTextFontSize + retryTextFontSize), menuTextFontSize, menuTextFontSize * BASE_FONT_SPACING, menuButtonIndex == 1 ? raylib::Color::Yellow() : raylib::Color::White());
	}

	//held piece
	std::string holdText = "HELD";
	float holdTextFontSize = FitTextWidth(mainFont, holdText, (UI_PIECE_LENGTH - 1.0f) * blockSize, BASE_FONT_SPACING);

	float holdHeight = blockSize * UI_PIECE_LENGTH + holdTextFontSize;

	//Drawing held piece
	{
		gridBackgroundColor.DrawRectangle({ fieldX, fieldY, blockSize * UI_PIECE_LENGTH, holdHeight });

		float holdTextWidth = mainFont.MeasureText(holdText, holdTextFontSize, holdTextFontSize / 10.0f).x;
		mainFont.DrawText(holdText, raylib::Vector2(gridX - (UI_PIECE_LENGTH - 0.5f) * blockSize, fieldY), holdTextFontSize, holdTextFontSize * BASE_FONT_SPACING, hasSwitchedPiece ? raylib::Color::Red() : raylib::Color::White());

		//Held grid lines

		//Horizontal grid lines
		for (int y = 0; y <= UI_PIECE_LENGTH; y++)
		{
			raylib::Color holdGridLineColor = hasSwitchedPiece ? raylib::Color::Red() : borderColor;
			holdGridLineColor.Alpha(y % (UI_PIECE_LENGTH) == 0 ? 0.6f : 0.2f).DrawLine(Vector2{ fieldX, fieldY + holdTextFontSize + y * blockSize }, Vector2{ fieldX + blockSize * UI_PIECE_LENGTH, fieldY + holdTextFontSize + y * blockSize }, (int)(3.0f * aspectScale));
		}

		//Vertical grid lines
		for (int x = 1; x < UI_PIECE_LENGTH; x++)
		{
			raylib::Color holdGridLineColor = hasSwitchedPiece ? raylib::Color::Red() : borderColor;
			holdGridLineColor.Alpha(0.2f).DrawLine(Vector2{ fieldX + x * blockSize, fieldY + holdTextFontSize }, Vector2{ fieldX + x * blockSize, fieldY + holdHeight }, (int)(3.0f * aspectScale));
		}

		float holdPieceStartX = gridX - 4 * blockSize;
		for (int i = 0; i < holdingPiece.numBlocks; i++)
		{
			raylib::Rectangle rect = { holdPieceStartX + blockSize * (holdingPiece.blockOffsets[i].x + 1), fieldY + holdTextFontSize + blockSize * (holdingPiece.blockOffsets[i].y + 1), blockSize, blockSize };

			blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, hasSwitchedPiece ? holdingPiece.blockColors[i].Alpha(0.5f) : holdingPiece.blockColors[i]);
		}
	}

	//up and coming pieces
	std::string nextText = "NEXT";
	float nextTextFontSize = FitTextWidth(mainFont, nextText, (UI_PIECE_LENGTH - 1.0f) * blockSize, BASE_FONT_SPACING);

	float nextHeight = blockSize * (UI_PIECE_LENGTH) * gameOptions.NumUpAndComingPieces + nextTextFontSize;

	//Drawing next pieces
	{
		gridBackgroundColor.DrawRectangle({ gridX + gridSize.x, fieldY, blockSize * UI_PIECE_LENGTH, nextHeight });

		float nextTextWidth = mainFont.MeasureText(nextText, nextTextFontSize, nextTextFontSize * BASE_FONT_SPACING).x;
		mainFont.DrawText(nextText, raylib::Vector2(gridX + gridSize.x + 0.5f * blockSize, fieldY), nextTextFontSize, nextTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

		//Next grid lines

		//Horizontal grid lines
		for (int y = 0; y < 3 * UI_PIECE_LENGTH; y++)
		{
			borderColor.Alpha(y % (UI_PIECE_LENGTH) == 0 ? 0.6f : 0.2f).DrawLine(Vector2{gridX + gridSize.x, fieldY + nextTextFontSize + y * blockSize}, Vector2{gridX + gridSize.x + blockSize * UI_PIECE_LENGTH, fieldY + nextTextFontSize + y * blockSize}, (int)(3.0f * aspectScale));
		}

		//Vertical grid lines
		for (int x = 1; x < UI_PIECE_LENGTH; x++)
		{
			borderColor.Alpha(0.2f).DrawLine(Vector2{ gridX + gridSize.x + x * blockSize, fieldY + nextTextFontSize }, Vector2{ gridX + gridSize.x + x * blockSize, fieldY + nextHeight }, (int)(3.0f * aspectScale));
		}

		//Pieces
		float nextPieceStartX = gridX + gridSize.x;
		for (int pieceIndex = 0; pieceIndex < gameOptions.NumUpAndComingPieces; pieceIndex++)
		{
			float pieceStartY = fieldY + nextTextFontSize + UI_PIECE_LENGTH * blockSize * pieceIndex;

			for (int i = 0; i < upAndComingPieces[pieceIndex].numBlocks; i++)
			{
				raylib::Rectangle rect = { nextPieceStartX + blockSize * (upAndComingPieces[pieceIndex].blockOffsets[i].x + 1), pieceStartY + blockSize * (upAndComingPieces[pieceIndex].blockOffsets[i].y + 1), blockSize, blockSize };

				blockTexture.Draw(blockTextureSource, rect, { 0.0f, 0.0f }, 0.0f, upAndComingPieces[pieceIndex].blockColors[i]);
			}
		}

	}

	//Statistics
	int statPanelHeightPadding = 10;
	float statTextFontSize = FitTextWidth(mainFont, "AAAAA", (UI_PIECE_LENGTH - 1.0f) * blockSize, BASE_FONT_SPACING);

	//Drawing statPanel
	{
		gridBackgroundColor.DrawRectangle({ fieldX, fieldY + holdHeight, blockSize * UI_PIECE_LENGTH, statTextFontSize * 8.0f + statPanelHeightPadding * 2.0f });
	
		//Score
		std::string scoreText = "SCORE";
		mainFont.DrawText(scoreText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

		mainFont.DrawText(TextFormat("%06i", score), raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

		//Level
		std::string levelText = "LEVEL";
		mainFont.DrawText(levelText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 2 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

		mainFont.DrawText(TextFormat("%03i", level), raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 3 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());
	
		//Cleared lines
		std::string clearedText = "LINES";
		mainFont.DrawText(clearedText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 4 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());
	
		mainFont.DrawText(TextFormat("%03i", totalLinesCleared), raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 5 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

		//Time
		std::string timeText = "TIME";
		mainFont.DrawText(timeText, raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 6 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());

		mainFont.DrawText(TextFormat("%03.0f", timePlayingSeconds), raylib::Vector2(fieldX + 0.5f * blockSize, fieldY + holdTextFontSize + blockSize * UI_PIECE_LENGTH + statTextFontSize * 7 + statPanelHeightPadding), statTextFontSize, statTextFontSize * BASE_FONT_SPACING, raylib::Color::White());
	}

	//TODO: should the lines here be moved into each part's own sections instead of all be clumped here?
	//Borders
	{
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

	int bagIndex = GetRandomValue(0, (int)bagPieces.size() - 1);

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
	for (int i = 0; i < gameOptions.NumUpAndComingPieces - 1; i++)
		upAndComingPieces[i] = upAndComingPieces[i + 1];

	//fill last spot with random piece from bag
	upAndComingPieces[gameOptions.NumUpAndComingPieces - 1] = GetRandomPieceFromBag();

	currentPiecePosition = { gameOptions.GridSize.x / 2 , 0 };

	gravityPieceDeltaTime = 0.0f;

	std::cout << "next piece in line" << std::endl;
}

void SceneGame::PlacePiece()
{
	int topPieceY = INT32_MAX;
	int bottomPieceY = INT32_MIN;

	//Place piece into grid
	for (int i = 0; i < currentPiece.numBlocks; i++)
	{
		if (!IsCellInBounds(currentPiecePosition.x + currentPiece.blockOffsets[i].x, currentPiecePosition.y + currentPiece.blockOffsets[i].y))
			continue;

		if (currentPiece.blockOffsets[i].y > bottomPieceY)
			bottomPieceY = currentPiece.blockOffsets[i].y;

		if (currentPiece.blockOffsets[i].y < topPieceY)
			topPieceY = currentPiece.blockOffsets[i].y;

		grid[currentPiecePosition.y + currentPiece.blockOffsets[i].y][currentPiecePosition.x + currentPiece.blockOffsets[i].x].color = currentPiece.blockColors[i];
		grid[currentPiecePosition.y + currentPiece.blockOffsets[i].y][currentPiecePosition.x + currentPiece.blockOffsets[i].x].state = BLOCK_GRID;
	}

	hasSwitchedPiece = false;

	std::cout << "Placed piece!" << std::endl;

	//Checks for cleared lines
	LineClearCheck(currentPiecePosition.y + topPieceY, currentPiecePosition.y + bottomPieceY);

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

	currentPiecePosition = { gameOptions.GridSize.x / 2 , 0 };
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

void SceneGame::UpdatePieceRotation()
{
	Piece piece = currentPiece;

	//rotation
	if (IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL) || IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_E))
		piece = piece.GetCounterClockwiseRotation(); //Counter-clockwise
	else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_X) || IsKeyPressed(KEY_R) || IsKeyPressed(KEY_W))
		piece = piece.GetClockwiseRotation(); //Clockwise
	else if (IsKeyPressed(KEY_T))
		piece = piece.GetHalfCircleRotation();
	else
		return;

	//If rotated piece can exist here, set current piece to rotated form of current piece without moving it
	if (CanPieceExistAt(piece, currentPiecePosition))
	{
		currentPiece = piece;
		return;
	}

	//Slightly move the piece if necessary and possible, allowing for rotation when near the border or other blocks without getting stuck
	//The amount of blocks that the piece needs to be moved depends on 
	//how many columns (x) contained in non-empty spots if the piece had been rotated without being moved at all

	int leftNonEmptyX = INT32_MAX;
	int rightNonEmptyX = INT32_MIN;

	for (int i = 0; i < piece.numBlocks; i++)
	{
		if (IsCellEmpty(currentPiecePosition.x + piece.blockOffsets[i].x, currentPiecePosition.y + piece.blockOffsets[i].y))
			continue;

		if (piece.blockOffsets[i].x < leftNonEmptyX)
			leftNonEmptyX = piece.blockOffsets[i].x;

		if (piece.blockOffsets[i].x > rightNonEmptyX)
			rightNonEmptyX = piece.blockOffsets[i].x;
	}

	int nonEmptyWidth = abs(leftNonEmptyX - rightNonEmptyX) + 1;

	if (CanPieceExistAt(piece, Vector2Int{ currentPiecePosition.x - nonEmptyWidth, currentPiecePosition.y }))
	{
		currentPiecePosition = Vector2Int{ currentPiecePosition.x - nonEmptyWidth, currentPiecePosition.y };
		currentPiece = piece;
	}
	else if (CanPieceExistAt(piece, Vector2Int{ currentPiecePosition.x + nonEmptyWidth, currentPiecePosition.y }))
	{
		currentPiecePosition = Vector2Int{ currentPiecePosition.x + nonEmptyWidth, currentPiecePosition.y };
		currentPiece = piece;
	}
}

void SceneGame::UpdatePieceMovement()
{
	//movement
	Vector2Int movement = { 0, 0 };
	const float movePieceTime = 1.0f / 10.0f;

	if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
	{
		movement.x = 1;

		if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
		{
			if (CanPieceExistAt(currentPiece, Vector2Int{ currentPiecePosition.x + 1, currentPiecePosition.y }))
				currentPiecePosition = Vector2Int{ currentPiecePosition.x + 1, currentPiecePosition.y };

			movementPieceDeltaTime = -movePieceTime; //extra delay before repeated movements
		}
	}
	else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
	{
		movement.x = -1;

		if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
		{
			if (CanPieceExistAt(currentPiece, Vector2Int{ currentPiecePosition.x - 1, currentPiecePosition.y }))
				currentPiecePosition = Vector2Int{ currentPiecePosition.x - 1, currentPiecePosition.y };

			movementPieceDeltaTime = -movePieceTime; //extra delay before repeated movements
		}
	}

	if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) && movementPieceDeltaTime >= movePieceTime)
	{
		while (movementPieceDeltaTime >= movePieceTime)
		{
			movementPieceDeltaTime -= movePieceTime;

			Vector2Int newPiecePosition = { currentPiecePosition.x + movement.x, currentPiecePosition.y + movement.y };

			if (CanPieceExistAt(currentPiece, newPiecePosition))
				currentPiecePosition = newPiecePosition;
			else
				break;
		}
	}
}

void SceneGame::UpdatePieceGravity()
{
	if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
		gravityPieceDeltaTime = 1.0f / 20.0f;

	int gravityLevel = std::min(level - 1, 14);

	float gravityMovementTime = (float)std::pow(0.8f - ((float)gravityLevel * 0.007f), (float)gravityLevel);

	//Soft drop speed
	if ((IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) && gravityMovementTime > 1.0f / 20.0f)
		gravityMovementTime = 1.0f / 20.0f;

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

#pragma endregion

#pragma region Grid

bool SceneGame::IsCellInBounds(int x, int y) const
{
	return x >= 0 && x < gameOptions.GridSize.x && y >= 0 && y < gameOptions.GridSize.y;
}

//Out of bounds cells do not count as empty and thus return false, unless this cell is above the grid but still within the left and right bounds.
bool SceneGame::IsCellEmpty(int x, int y)
{
	if (x < 0 || x >= gameOptions.GridSize.x || y >= gameOptions.GridSize.y)
		return false;

	if (y < 0)
		return true;

	return grid[y][x].state == BLOCK_EMPTY;
}

void SceneGame::SetGridSize(Vector2Int gridSize)
{
	//Delete old grid
	if (grid != nullptr)
	{
		for (int y = 0; y < gameOptions.GridSize.y; y++)
			delete[] grid[y];

		delete[] grid;
	}

	gameOptions.GridSize = gridSize;

	//Create grid
	grid = new BlockCell * [gridSize.y];
	for (int y = 0; y < gridSize.y; y++)
	{
		grid[y] = new BlockCell[gridSize.x];

		for (int x = 0; x < gridSize.x; x++)
		{
			grid[y][x] = BlockCell(BLOCK_EMPTY, raylib::Color::Blank());
		}
	}
}

void SceneGame::ClearLine(int line)
{
	//shift everything downwards
	for (int y = line; y > 0; y--)
	{
		for (int x = 0; x < gameOptions.GridSize.x; x++)
		{
			grid[y][x] = grid[y - 1][x];
		}
	}

	//clear line 0
	for (int x = 0; x < gameOptions.GridSize.x; x++)
		grid[0][x].state = BLOCK_EMPTY;
	
	std::cout << "Cleared line " + std::to_string(line) << std::endl;
}

void SceneGame::DrawGrid(float posX, float posY, float blockSize, raylib::Texture2D& blockTexture)
{
	raylib::Rectangle blockTextureSource = { 0.0f, 0.0f, (float)blockTexture.width, (float)blockTexture.height };

	//Draw grid cells
	for (int gridY = 0; gridY < gameOptions.GridSize.y; gridY++)
	{
		for (int gridX = 0; gridX < gameOptions.GridSize.x; gridX++)
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
					const int FLASH_LENGTH = std::min(6, gameOptions.GridSize.x);

					float startFlashTime = (lineClearTimeSeconds - (lineClearTimeSeconds / gameOptions.GridSize.x) * (FLASH_LENGTH - 2)) / gameOptions.GridSize.x * (gridX);
					float endFlashTime = (lineClearTimeSeconds - (lineClearTimeSeconds / gameOptions.GridSize.x) * (FLASH_LENGTH - 2)) / gameOptions.GridSize.x * (gridX + FLASH_LENGTH);

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

		if (gameOptions.ShowGhostPiece && currentPiece.numBlocks != 0)
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
void SceneGame::UpdateMenuButtonNagivation(int startIndex, int endIndex)
{
	if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
	{
		menuButtonIndex += 1;

		if (menuButtonIndex > endIndex)
			menuButtonIndex = startIndex;
	}
	else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
	{
		menuButtonIndex -= 1;

		if (menuButtonIndex < 0)
			menuButtonIndex = endIndex;
	}
}

void SceneGame::UpdateTitleMenu()
{

#ifdef PLATFORM_WEB
	//get rid of the quit button
	int numButtons = 4;
#else
	int numButtons = 5;
#endif

	UpdateMenuButtonNagivation(0, numButtons - 1);

	switch (menuButtonIndex)
	{
		//start button
		case 0:
			if (IsConfirmButtonPressed())
			{
				menuState = MENU_NONE;
				StartGame();
			}
			break;
		//options button
		case 1:
			if (IsConfirmButtonPressed())
			{
				menuState = MENU_OPTIONS;
				menuButtonIndex = 0;
			}
			break;
		//controls button
		case 2:
			if (IsConfirmButtonPressed())
			{
				menuState = MENU_CONTROLS;
				menuButtonIndex = 0;
			}
			break;
		//credits button
		case 3:
			if (IsConfirmButtonPressed())
			{
				menuState = MENU_CREDITS;
				menuButtonIndex = 0;
			}
			break;
		//quit button
		case 4:
			if (IsConfirmButtonPressed())
			{
				WantsToQuit = true;
			}
			break;
	}
}

void SceneGame::UpdateOptionsMenu()
{
	UpdateMenuButtonNagivation(0, 5);

	switch (menuButtonIndex)
	{
		//play music option
		case 0:
			if (IsConfirmButtonPressed())
			{
				gameOptions.PlayMusic = !gameOptions.PlayMusic;
			}
			break;
		//strobing lights option
		case 1:
			if (IsConfirmButtonPressed())
			{
				gameOptions.EnableStrobingLights = !gameOptions.EnableStrobingLights;
			}
			break;
		//grid width option
		case 2:
		{
			const int MAX_GRID_WIDTH = 30;
			const int MIN_GRID_WIDTH = 3;

			if (IsConfirmButtonPressed() || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
			{
				if (gameOptions.GridSize.x + 1 > MAX_GRID_WIDTH)
					SetGridSize(Vector2Int{ MIN_GRID_WIDTH, gameOptions.GridSize.y });
				else
					SetGridSize(Vector2Int{ gameOptions.GridSize.x + 1, gameOptions.GridSize.y });
			}
			else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
			{
				if (gameOptions.GridSize.x - 1 < MIN_GRID_WIDTH)
					SetGridSize(Vector2Int{ MAX_GRID_WIDTH, gameOptions.GridSize.y });
				else
					SetGridSize(Vector2Int{ gameOptions.GridSize.x - 1, gameOptions.GridSize.y });
			}

			break;
		}
		//grid height option
		case 3:
		{
			const int MAX_GRID_HEIGHT = 60;
			const int MIN_GRID_HEIGHT = 16;

			if (IsConfirmButtonPressed() || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
			{
				if (gameOptions.GridSize.y + 1 > MAX_GRID_HEIGHT)
					SetGridSize(Vector2Int{ gameOptions.GridSize.x, MIN_GRID_HEIGHT });
				else
					SetGridSize(Vector2Int{ gameOptions.GridSize.x, gameOptions.GridSize.y + 1 });
			}
			else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
			{
				if (gameOptions.GridSize.y - 1 < MIN_GRID_HEIGHT)
					SetGridSize(Vector2Int{ gameOptions.GridSize.x, MAX_GRID_HEIGHT });
				else
					SetGridSize(Vector2Int{ gameOptions.GridSize.x, gameOptions.GridSize.y - 1 });
			}

			break;
		}
		//ghost piece option
		case 4:
			if (IsConfirmButtonPressed())
			{
				gameOptions.ShowGhostPiece = !gameOptions.ShowGhostPiece;
			}
			break;
		//back button
		case 5:
			if (IsConfirmButtonPressed())
			{
				menuState = MENU_TITLE;
				menuButtonIndex = 1;
			}
			break;
	}
}

void SceneGame::UpdateControlsMenu()
{
	if (IsConfirmButtonPressed())
	{
		menuState = MENU_TITLE;
		menuButtonIndex = 2;
	}
}

void SceneGame::UpdateCreditsMenu()
{
	UpdateMenuButtonNagivation(0, 3);

	switch (menuButtonIndex)
	{
		//yt link
		case 0:
			if (IsConfirmButtonPressed())
			{
				raylib::OpenURL("https://www.youtube.com/channel/UCVjBKRRHM1u8FYEnmt6JG1g");
			}
			break;
		//kevin macleod link
		case 1:
			if (IsConfirmButtonPressed())
			{
				raylib::OpenURL("https://incompetech.com/");
			}
			break;
		//creative commons link
		case 2:
			if (IsConfirmButtonPressed())
			{
				raylib::OpenURL("http://creativecommons.org/licenses/by/3.0/");
			}
			break;
		//back button
		case 3:
			if (IsConfirmButtonPressed())
			{
				menuState = MENU_TITLE;
				menuButtonIndex = 3;
				break;
			}
	}
}

void SceneGame::DrawTitleMenu() 
{
	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	float aspectScale = std::min((float)screenWidth / DESIGN_WIDTH, (float)screenHeight / DESIGN_HEIGHT);

	raylib::Font& mainFont = GetFont("MainFont");

	//Background
	raylib::Color backgroundColor = raylib::Color::Black().Alpha(0.6f);
	backgroundColor.DrawRectangle(0, 0, screenWidth, screenHeight);

	//Icon
	float iconScale = (1.0f + (sinf((float)gameWindow.GetTime())) * 0.05f) * aspectScale;
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

		mainFont.DrawText(std::string(1, titleText.at(i)), raylib::Vector2(titleTextX, screenHeight / 2.0f - (float)iconTexture.height * iconScale * 1.5f - titleTextSize + sinf((float)gameWindow.GetTime() * 3.0f + i) * 4.0f * aspectScale), titleTextSize, titleTextSize * BASE_FONT_SPACING, raylib::Color::FromHSV(Wrap((float)gameWindow.GetTime() * 120.0f + 30.0f * i, 0.0f, 360.0f), 1.0f, 1.0f));
		
		float titleCharWidth = mainFont.MeasureText(titleChar, titleTextSize, titleTextSize * BASE_FONT_SPACING).x;
		titleTextX += titleCharWidth + (titleTextSize / 10);
	}

	//Buttons
	float buttonTextSize = 48 * aspectScale;

	std::string startText = "START";
	float startWidth = mainFont.MeasureText(startText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(startText, raylib::Vector2(screenWidth / 2.0f - startWidth / 2.0f, screenHeight / 2.0f - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 0 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	std::string optionsText = "OPTIONS";
	float optionsWidth = mainFont.MeasureText(optionsText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(optionsText, raylib::Vector2(screenWidth / 2.0f - optionsWidth / 2.0f, screenHeight / 2.0f + buttonTextSize - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 1 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	std::string controlsText = "CONTROLS";
	float controlsWidth = mainFont.MeasureText(controlsText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(controlsText, raylib::Vector2(screenWidth / 2.0f - controlsWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 2 - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 2 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	std::string creditsText = "CREDITS";
	float creditsWidth = mainFont.MeasureText(creditsText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(creditsText, raylib::Vector2(screenWidth / 2.0f - creditsWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 3 - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 3 ? raylib::Color::Yellow() : raylib::Color::LightGray());

#ifndef PLATFORM_WEB
	std::string quitText = "QUIT";
	float quitWidth = mainFont.MeasureText(quitText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(quitText, raylib::Vector2(screenWidth / 2.0f - quitWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 4 - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 4 ? raylib::Color::Yellow() : raylib::Color::LightGray());
#endif // !PLATFORM_WEB

	DrawBuildInfo();

	//Help text
	float helpTextSize = 12 * aspectScale;
	std::string helpText = "MOVE UP - Up/W | MOVE DOWN - Down/S | CONFIRM - Space/Enter";

	float helpTextWidth = mainFont.MeasureText(helpText, helpTextSize, helpTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(helpText, raylib::Vector2(screenWidth / 2.0f - helpTextWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 5 + 6 * aspectScale - buttonTextSize / 2.0f), helpTextSize, helpTextSize * BASE_FONT_SPACING, raylib::Color::White());
}

void SceneGame::DrawOptionsMenu()
{
	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	float aspectScale = std::min((float)screenWidth / DESIGN_WIDTH, (float)screenHeight / DESIGN_HEIGHT);

	raylib::Font& mainFont = GetFont("MainFont");

	//Background
	raylib::Color backgroundColor = raylib::Color::Black().Alpha(0.6f);
	backgroundColor.DrawRectangle(0, 0, screenWidth, screenHeight);

	//Icon
	float iconScale = (1.0f + (sinf((float)gameWindow.GetTime())) * 0.05f) * aspectScale;
	raylib::Texture2D& iconTexture = GetTexture("Icon");
	raylib::Rectangle iconSourceRect = raylib::Rectangle{ 0, 0, (float)iconTexture.width, (float)iconTexture.height };
	iconTexture.Draw(iconSourceRect, raylib::Rectangle{ screenWidth / 2.0f, screenHeight / 2.0f - (float)iconTexture.height * iconScale, (float)iconTexture.width * iconScale, (float)iconTexture.height * iconScale }, { (float)iconTexture.width / 2.0f * iconScale, (float)iconTexture.height / 2.0f * iconScale }, 0.0f, raylib::Color::White());

	//Options title
	std::string titleText = "OPTIONS";
	float titleTextSize = 80 * aspectScale;
	float titleWidth = mainFont.MeasureText(titleText, titleTextSize, titleTextSize * BASE_FONT_SPACING).x;
	float titleTextX = (screenWidth - titleWidth) / 2.0f;

	for (int i = 0; i < titleText.length(); i++)
	{
		std::string titleChar = std::string(1, titleText.at(i));

		mainFont.DrawText(std::string(1, titleText.at(i)), raylib::Vector2(titleTextX, screenHeight / 2.0f - (float)iconTexture.height * iconScale * 1.5f - titleTextSize + sinf((float)gameWindow.GetTime() * 3.0f + i) * 4.0f * aspectScale), titleTextSize, titleTextSize * BASE_FONT_SPACING, raylib::Color::FromHSV(Wrap((float)gameWindow.GetTime() * 120.0f + 30.0f * i, 0.0f, 360.0f), 1.0f, 1.0f));

		float titleCharWidth = mainFont.MeasureText(titleChar, titleTextSize, titleTextSize * BASE_FONT_SPACING).x;
		titleTextX += titleCharWidth + (titleTextSize / 10);
	}

	//Options
	float optionTextSize = 40 * aspectScale;

	//MUSIC
	std::string musicText = "MUSIC: ";
	musicText += gameOptions.PlayMusic ? "ON" : "OFF";

	float musicTextWidth = mainFont.MeasureText(musicText, optionTextSize, optionTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(musicText, raylib::Vector2(screenWidth / 2.0f - musicTextWidth / 2.0f, screenHeight / 2.0f - optionTextSize / 2.0f), optionTextSize, optionTextSize * BASE_FONT_SPACING, menuButtonIndex == 0 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	//Strobing lights
	std::string strobingLightsText = "STROBING LIGHTS: ";
	strobingLightsText += gameOptions.EnableStrobingLights ? "ON" : "OFF";

	float strobingLightsWidth = mainFont.MeasureText(strobingLightsText, optionTextSize, optionTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(strobingLightsText, raylib::Vector2(screenWidth / 2.0f - strobingLightsWidth / 2.0f, screenHeight / 2.0f + optionTextSize - optionTextSize / 2.0f), optionTextSize, optionTextSize * BASE_FONT_SPACING, menuButtonIndex == 1 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	//Width
	float widthTextWidth = mainFont.MeasureText(TextFormat("WIDTH: < %i >", gameOptions.GridSize.x), optionTextSize, optionTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(TextFormat("WIDTH: < %i >", gameOptions.GridSize.x), raylib::Vector2(screenWidth / 2.0f - widthTextWidth / 2.0f, screenHeight / 2.0f + optionTextSize * 2 - optionTextSize / 2.0f), optionTextSize, optionTextSize * BASE_FONT_SPACING, menuButtonIndex == 2 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	//Height
	float heightTextWidth = mainFont.MeasureText(TextFormat("HEIGHT: < %i >", gameOptions.GridSize.y), optionTextSize, optionTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(TextFormat("HEIGHT: < %i >", gameOptions.GridSize.y), raylib::Vector2(screenWidth / 2.0f - heightTextWidth / 2.0f, screenHeight / 2.0f + optionTextSize * 3 - optionTextSize / 2.0f), optionTextSize, optionTextSize * BASE_FONT_SPACING, menuButtonIndex == 3 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	//Show ghost piece
	std::string ghostPieceText = "GHOST PIECE: ";
	ghostPieceText += gameOptions.ShowGhostPiece ? "ON" : "OFF";

	float ghostPieceTextWidth = mainFont.MeasureText(ghostPieceText, optionTextSize, optionTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(ghostPieceText, raylib::Vector2(screenWidth / 2.0f - ghostPieceTextWidth / 2.0f, screenHeight / 2.0f + optionTextSize * 4 - optionTextSize / 2.0f), optionTextSize, optionTextSize * BASE_FONT_SPACING, menuButtonIndex == 4 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	//Buttons
	float buttonTextSize = 52 * aspectScale;

	std::string backText = "BACK";
	float backWidth = mainFont.MeasureText(backText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(backText, raylib::Vector2(screenWidth / 2.0f - backWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 4 - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 5 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	DrawBuildInfo();
}

void SceneGame::DrawControlsMenu()
{
	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	float aspectScale = std::min((float)screenWidth / DESIGN_WIDTH, (float)screenHeight / DESIGN_HEIGHT);

	raylib::Font& mainFont = GetFont("MainFont");

	//Background
	raylib::Color backgroundColor = raylib::Color::Black().Alpha(0.6f);
	backgroundColor.DrawRectangle(0, 0, screenWidth, screenHeight);

	//Icon
	float iconScale = (1.0f + (sinf((float)gameWindow.GetTime())) * 0.05f) * aspectScale;
	raylib::Texture2D& iconTexture = GetTexture("Icon");
	raylib::Rectangle iconSourceRect = raylib::Rectangle{ 0, 0, (float)iconTexture.width, (float)iconTexture.height };
	iconTexture.Draw(iconSourceRect, raylib::Rectangle{ screenWidth / 2.0f, screenHeight / 2.0f - (float)iconTexture.height * iconScale, (float)iconTexture.width * iconScale, (float)iconTexture.height * iconScale }, { (float)iconTexture.width / 2.0f * iconScale, (float)iconTexture.height / 2.0f * iconScale }, 0.0f, raylib::Color::White());

	//controls title
	std::string titleText = "CONTROLS";
	float titleTextSize = 80 * aspectScale;
	float titleWidth = mainFont.MeasureText(titleText, titleTextSize, titleTextSize * BASE_FONT_SPACING).x;
	float titleTextX = (screenWidth - titleWidth) / 2.0f;

	for (int i = 0; i < titleText.length(); i++)
	{
		std::string titleChar = std::string(1, titleText.at(i));

		mainFont.DrawText(std::string(1, titleText.at(i)), raylib::Vector2(titleTextX, screenHeight / 2.0f - (float)iconTexture.height * iconScale * 1.5f - titleTextSize + sinf((float)gameWindow.GetTime() * 3.0f + (float)i) * 4.0f * aspectScale), titleTextSize, titleTextSize * BASE_FONT_SPACING, raylib::Color::FromHSV(Wrap((float)gameWindow.GetTime() * 120.0f + 30.0f * i, 0.0f, 360.0f), 1.0f, 1.0f));

		float titleCharWidth = mainFont.MeasureText(titleChar, titleTextSize, titleTextSize * BASE_FONT_SPACING).x;
		titleTextX += titleCharWidth + (titleTextSize / 10);
	}

	//Controls
	float controlsTextSize = 24 * aspectScale;

	std::string controlsText = "LEFT - Left/A | RIGHT - Right/D\nCLOCKWISE ROTATE - Up/W/X/R\nCOUNTER-CLOCKWISE ROTATE - L Ctrl/R Ctrl/Z/E\n180 DEG ROTATE - T\nSOFT DROP - Down/S\nHARD DROP/CONFIRM - Space/Enter\nHOLD - C/Left Shift/Right Shift\nPAUSE - ESC/F1";
	int lineY = 0;

	//Draw every line aligned along the center of the screen
	while (controlsText != "")
	{
		int endLineIndex = (int)controlsText.find('\n', 0);

		std::string controlText = "";
		
		//final line
		if (endLineIndex == std::string::npos)
		{
			controlText = controlsText;
			controlsText = "";
		}
		//Get next line and remove it
		else
		{
			controlText = controlsText.substr(0, endLineIndex);

			controlsText.erase(0, endLineIndex + 1);
		}

		//Draw line
		float controlTextWidth = mainFont.MeasureText(controlText, controlsTextSize, controlsTextSize * BASE_FONT_SPACING).x;
		mainFont.DrawText(controlText, raylib::Vector2(screenWidth / 2.0f - controlTextWidth / 2.0f, screenHeight / 2.0f + controlsTextSize * lineY - controlsTextSize / 2.0f), controlsTextSize, controlsTextSize * BASE_FONT_SPACING, raylib::Color::White());

		lineY++;
	}

	//Buttons
	float buttonTextSize = 52 * aspectScale;

	std::string backText = "BACK";
	float backWidth = mainFont.MeasureText(backText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(backText, raylib::Vector2(screenWidth / 2.0f - backWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 4 - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 0 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	DrawBuildInfo();
}

void SceneGame::DrawCreditsMenu()
{
	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	float aspectScale = std::min((float)screenWidth / DESIGN_WIDTH, (float)screenHeight / DESIGN_HEIGHT);

	raylib::Font& mainFont = GetFont("MainFont");

	//Background
	raylib::Color backgroundColor = raylib::Color::Black().Alpha(0.6f);
	backgroundColor.DrawRectangle(0, 0, screenWidth, screenHeight);

	//Icon
	float iconScale = (1.0f + (sinf((float)gameWindow.GetTime())) * 0.05f) * aspectScale;
	raylib::Texture2D& iconTexture = GetTexture("Icon");
	raylib::Rectangle iconSourceRect = raylib::Rectangle{ 0, 0, (float)iconTexture.width, (float)iconTexture.height };
	iconTexture.Draw(iconSourceRect, raylib::Rectangle{ screenWidth / 2.0f, screenHeight / 2.0f - (float)iconTexture.height * iconScale, (float)iconTexture.width * iconScale, (float)iconTexture.height * iconScale }, { (float)iconTexture.width / 2.0f * iconScale, (float)iconTexture.height / 2.0f * iconScale }, 0.0f, raylib::Color::White());

	//credits title
	std::string titleText = "CREDITS";
	float titleTextSize = 80 * aspectScale;
	float titleWidth = mainFont.MeasureText(titleText, titleTextSize, titleTextSize * BASE_FONT_SPACING).x;
	float titleTextX = (screenWidth - titleWidth) / 2.0f;

	for (int i = 0; i < titleText.length(); i++)
	{
		std::string titleChar = std::string(1, titleText.at(i));

		mainFont.DrawText(std::string(1, titleText.at(i)), raylib::Vector2(titleTextX, screenHeight / 2.0f - (float)iconTexture.height * iconScale * 1.5f - titleTextSize + sinf((float)gameWindow.GetTime() * 3.0f + i) * 4.0f * aspectScale), titleTextSize, titleTextSize * BASE_FONT_SPACING, raylib::Color::FromHSV(Wrap((float)gameWindow.GetTime() * 120.0f + 30.0f * i, 0.0f, 360.0f), 1.0f, 1.0f));

		float titleCharWidth = mainFont.MeasureText(titleChar, titleTextSize, titleTextSize * BASE_FONT_SPACING).x;
		titleTextX += titleCharWidth + (titleTextSize / 10);
	}

	//music credits
		//"Bleeping Demo", "Nowhere Land"
		//Kevin MacLeod(incompetech.com)
		//Licensed under Creative Commons : By Attribution 3.0
		//http://creativecommons.org/licenses/by/3.0/

	//credits
	float creditsTextSize = 28 * aspectScale;

	std::string recreationText = "RECREATION - KiaraDev (YouTube)";
	float recreationTextWidth = mainFont.MeasureText(recreationText, creditsTextSize, creditsTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(recreationText, raylib::Vector2(screenWidth / 2.0f - recreationTextWidth / 2.0f, screenHeight / 2.0f - creditsTextSize / 2.0f), creditsTextSize, creditsTextSize * BASE_FONT_SPACING, menuButtonIndex == 0 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	std::string musicCreditsText1 = "\"Bleeping Demo\", \"Nowhere Land\"";
	float musicCreditsTextWidth1 = mainFont.MeasureText(musicCreditsText1, creditsTextSize, creditsTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(musicCreditsText1, raylib::Vector2(screenWidth / 2.0f - musicCreditsTextWidth1 / 2.0f, screenHeight / 2.0f + creditsTextSize * 2 - creditsTextSize / 2.0f), creditsTextSize, creditsTextSize * BASE_FONT_SPACING, raylib::Color::White());

	std::string musicCreditsText2 = "Kevin MacLeod(incompetech.com)";
	float musicCreditsTextWidth2 = mainFont.MeasureText(musicCreditsText2, creditsTextSize, creditsTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(musicCreditsText2, raylib::Vector2(screenWidth / 2.0f - musicCreditsTextWidth2 / 2.0f, screenHeight / 2.0f + creditsTextSize * 3 - creditsTextSize / 2.0f), creditsTextSize, creditsTextSize * BASE_FONT_SPACING, menuButtonIndex == 1 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	std::string musicCreditsText3 = "Licensed under Creative Commons : By Attribution 3.0";
	float musicCreditsTextWidth3 = mainFont.MeasureText(musicCreditsText3, creditsTextSize, creditsTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(musicCreditsText3, raylib::Vector2(screenWidth / 2.0f - musicCreditsTextWidth3 / 2.0f, screenHeight / 2.0f + creditsTextSize * 4 - creditsTextSize / 2.0f), creditsTextSize, creditsTextSize * BASE_FONT_SPACING, raylib::Color::White());

	std::string musicCreditsText4 = "http://creativecommons.org/licenses/by/3.0/";
	float musicCreditsTextWidth4 = mainFont.MeasureText(musicCreditsText4, creditsTextSize, creditsTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(musicCreditsText4, raylib::Vector2(screenWidth / 2.0f - musicCreditsTextWidth4 / 2.0f, screenHeight / 2.0f + creditsTextSize * 5 - creditsTextSize / 2.0f), creditsTextSize, creditsTextSize * BASE_FONT_SPACING, menuButtonIndex == 2 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	//Buttons
	float buttonTextSize = 52 * aspectScale;

	std::string backText = "BACK";
	float backWidth = mainFont.MeasureText(backText, buttonTextSize, buttonTextSize * BASE_FONT_SPACING).x;
	mainFont.DrawText(backText, raylib::Vector2(screenWidth / 2.0f - backWidth / 2.0f, screenHeight / 2.0f + buttonTextSize * 4 - buttonTextSize / 2.0f), buttonTextSize, buttonTextSize * BASE_FONT_SPACING, menuButtonIndex == 3 ? raylib::Color::Yellow() : raylib::Color::LightGray());

	DrawBuildInfo();
}

void SceneGame::DrawBuildInfo()
{
	int screenWidth = gameWindow.GetWidth();
	int screenHeight = gameWindow.GetHeight();

	float aspectScale = std::min((float)screenWidth / DESIGN_WIDTH, (float)screenHeight / DESIGN_HEIGHT);

	raylib::Font& mainFont = GetFont("MainFont");

	//Build info
	float buildInfoTextSize = 12 * aspectScale;
	mainFont.DrawText(VERSION + " - " + PLATFORM, raylib::Vector2(5 * aspectScale, screenHeight - buildInfoTextSize - 5 * aspectScale), buildInfoTextSize, buildInfoTextSize / 10.0f, raylib::Color::White());

#ifdef DEBUG
	mainFont.DrawText("DEBUG", raylib::Vector2(5 * aspectScale, screenHeight - buildInfoTextSize * 2 - 5 * aspectScale), buildInfoTextSize, buildInfoTextSize / 10.0f, raylib::Color::White());
#endif
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
		case MENU_CREDITS:
			DrawCreditsMenu();
			break;
		default:
			break;
	}

#if DEBUG
	//For debug purposes
	DrawText(TextFormat("Button index: %i", menuButtonIndex), 12, 12 + 24, 24, raylib::Color::White());
#endif
}

void SceneGame::Destroy()
{
	//Destroy grid

	for (int y = 0; y < gameOptions.GridSize.y; y++)
		delete[] grid[y];

	delete[] grid;
}
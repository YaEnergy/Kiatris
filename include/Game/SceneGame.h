#pragma once

#include "raylib-cpp.hpp"
#include "Scene.h"
#include "Game/BlockCell.h"
#include "Game/Piece.h"
#include "Game/GameModifiers.h"
#include <iostream>

enum MenuState
{
	MENU_TITLE,
	MENU_CONTROLS,
	MENU_OPTIONS,
	MENU_NONE
};

class SceneGame : public Scene
{
	private:
		raylib::Window& gameWindow;

		GameModifiers gameModifiers;

		Piece currentPiece;
		Vector2Int currentPiecePosition = { 0, 0 };
		float gravityPieceDeltaTime = 0.0f;
		float movementPieceDeltaTime = 0.0f;

		Piece holdingPiece;
		bool hasSwitchedPiece = false;

		std::vector<Piece> bagPieces;
		std::vector<Piece> upAndComingPieces;

		BlockCell** grid = nullptr;

		float lineClearTimeSeconds = 0.25f;
		float deltaLineClearingTime = 0.0f;
		bool isClearingLines = false;
		std::vector<int> clearingLines;

		bool gameOver = false;
		bool gamePaused = false;
		
		int menuButtonIndex = 0;
		MenuState menuState = MENU_TITLE;

		//statistics
		int score = 0;
		int level = 0;
		int totalLinesCleared = 0;
		float timePlayingSeconds = 0;

		//Gameplay
		void SetGameModifiers(GameModifiers modifiers);
		void StartGame();
		void UpdateGameplay();
		void UpdatePieceMovement();
		void UpdatePieceGravity();
		void LineClearCheck();
		void UpdateGameOver();
		void EndGame();
		void ReturnToMenu();

		void DrawGame();

		//Menus
		void UpdateTitleMenu();
		void UpdateOptionsMenu();
		void UpdateControlsMenu();
		
		void DrawTitleMenu();
		void DrawOptionsMenu();
		void DrawControlsMenu();
		void DrawPauseMenu();

		//Pieces

		Piece GetRandomPiece();
		bool CanPieceExistAt(Piece piece, Vector2Int position);

		void RefillBag();
		Piece GetRandomPieceFromBag();

		void NextPiece();
		void PlacePiece();
		void HoldPiece();
		void HardDropPiece();

		//Grid
		bool IsCellInBounds(int x, int y) const;
		bool IsCellEmpty(int x, int y); 
		void ClearLine(int line);
		void DrawGrid(float x, float y, float blockSize, raylib::Texture2D& blockTexture);

	public:
		SceneGame(raylib::Window& window, GameModifiers modifiers) : gameWindow(window)
		{
			SetGameModifiers(modifiers);
		}

		void Init();

		void Update();
		
		void Draw();

		void Destroy();
};
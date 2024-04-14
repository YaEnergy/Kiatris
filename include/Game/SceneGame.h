#pragma once

#include "raylib-cpp.hpp"
#include "Scene.h"
#include "Game/Piece.h"
#include "Game/GameModifiers.h"
#include <iostream>

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
		std::vector<Piece> upAndComingPieces;

		raylib::Color** grid = nullptr;

		bool gameOver = false;

		//statistics
		int score = 0;
		int level = 0;
		int totalLinesCleared = 0;
		float timePlayingSeconds = 0;

		//Gameplay
		void StartGame(GameModifiers modifiers);
		void UpdateGameplay();
		void UpdateGameOver();
		void EndGame();

		//Pieces
		Piece GetRandomPiece();
		bool CanPieceExistAt(Piece piece, Vector2Int position);

		void NextPiece();
		void PlacePiece();
		void HoldPiece();
		void HardDropPiece();

		//Grid
		bool IsCellEmpty(int x, int y); 
		void ClearLine(int line);

	public:
		SceneGame(raylib::Window& window, GameModifiers modifiers) : gameWindow(window)
		{
			StartGame(modifiers);
		}

		void Init();

		void Update();
		
		void Draw();

		void Destroy();
};
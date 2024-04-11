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

		Piece holdingPiece;
		std::vector<Piece> upAndComingPieces;

		raylib::Color** grid;

		bool gameOver = false;

		//statistics
		int score = 0;
		int level = 0;
		int linesCleared = 0;
		float timePlayingSeconds = 0;

		//Gameplay
		void UpdateGameplay();
		void UpdateGameOver();
		void EndGame();

		//Pieces
		Piece GetRandomPiece();
		bool CanPieceExistAt(Vector2Int position);

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
			gameModifiers = modifiers;

			//pieces
			upAndComingPieces = std::vector<Piece>(modifiers.NumUpAndComingPieces);
			for (int i = 0; i < modifiers.NumUpAndComingPieces; i++)
				upAndComingPieces[i] = GetRandomPiece();

			NextPiece();

			//Create grid
			grid = new raylib::Color*[modifiers.GridSize.y];
			for (int y = 0; y < modifiers.GridSize.y; y++)
			{
				grid[y] = new raylib::Color[modifiers.GridSize.x];

				for (int x = 0; x < modifiers.GridSize.x; x++)
				{
					grid[y][x] = raylib::Color::Blank();
				}
			}
		}

		void Init();

		void Update();
		
		void Draw();

		void Destroy();
};
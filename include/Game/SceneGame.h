#pragma once

#include "raylib-cpp.hpp"
#include "Scene.h"
#include "Game/Piece.h"
#include "Game/GameModifiers.h"
#include <iostream>

class SceneGame : public Scene
{
	private:
		GameModifiers gameModifiers;

		Piece currentPiece;
		Vector2Int currentPiecePosition = { 0, 0 };

		Piece holdingPiece;
		Piece* upAndComingPieces = nullptr;

		raylib::Color** grid;

		bool gameOver = false;

		//statistics
		int score = 0;
		int linesCleared = 0;
		float timePlayingSeconds = 0;

		//Gameplay
		void UpdateGameplay();
		void UpdateGameOver();
		void EndGame();

		//Pieces
		bool CanMovePiece(Vector2Int movement);
		bool CanRotatePieceLeft();
		bool CanRotatePieceRight();
		bool CanRotatePieceHalfCircle();

		void PlacePiece();
		void HoldPiece();
		void HardDropPiece();

		//Grid
		void ClearLine();

	public:
		SceneGame(GameModifiers modifiers)
		{
			gameModifiers = modifiers;

			//TODO: randomize pieces
			//current piece
			currentPiece = Piece::GetMainPiece(PIECE_T);
			
			//up and coming pieces
			upAndComingPieces = new Piece[modifiers.NumUpAndComingPieces];
			for (int i = 0; i < modifiers.NumUpAndComingPieces; i++)
				upAndComingPieces[i] = Piece::GetMainPiece(PIECE_I);

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
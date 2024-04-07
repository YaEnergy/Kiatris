#pragma once

#include "raylib-cpp.hpp"
#include "Scene.h"
#include "Game/Piece.h"
#include "Game/GameModifiers.h"

class SceneGame : public Scene
{
	private:
		GameModifiers gameModifiers;

		Piece currentPiece;
		Piece holdingPiece;
		Piece* upAndComingPieces;

		raylib::Color** grid;

		int score = 0;

	public:
		SceneGame(GameModifiers modifiers)
		{
			gameModifiers = modifiers;

			//TODO: randomize pieces
			//current piece
			currentPiece = Piece::GetMainPiece(PIECE_I);

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
#pragma once

#include "raylib-cpp.hpp"
#include "Vector2Int.h"

enum MainPieceType
{
	PIECE_O = 0,
	PIECE_I = 1,
	PIECE_S = 2,
	PIECE_Z = 3,
	PIECE_L = 4,
	PIECE_J = 5,
	PIECE_T = 6
};

struct Piece
{
	Vector2Int* blockOffsets;
	raylib::Color* blockColors;
	int numBlocks;

	void RotateLeft();
	void RotateRight();
	void RotateHalfCircle();

	Vector2Int Measure() const;
	raylib::Rectangle GetBounds() const;

	static Piece GetMainPiece(MainPieceType mainPieceType);

	Piece()
	{
		blockOffsets = nullptr;
		blockColors = nullptr;
		numBlocks = 0;
	}

	Piece(int numBlocks)
	{
		blockOffsets = new Vector2Int[numBlocks];
		blockColors = new raylib::Color[numBlocks];
		this->numBlocks = numBlocks;
	}

	Piece(Vector2Int blockOffsets[], raylib::Color blockColors[], int numBlocks)
	{
		this->blockOffsets = blockOffsets;
		this->blockColors = blockColors;
		this->numBlocks = numBlocks;
	}

	~Piece()
	{
		delete[] blockOffsets;
		delete[] blockColors;
	}
};
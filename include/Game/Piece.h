#pragma once

#include "raylib-cpp.hpp"
#include "Vector2Int.h"

struct Piece
{
	Vector2Int* blockOffsets;
	raylib::Color* blockColors;
	int numBlocks;

	void RotateLeft();
	void RotateRight();
	void RotateHalfCircle();

	Vector2Int Measure() const;

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
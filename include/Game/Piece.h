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
	raylib::Vector2 pivotOffset; //pivot to rotate around
	Vector2Int* blockOffsets; //block offsets from origin, not pivot
	raylib::Color* blockColors; //colors of blocks
	int numBlocks; //amount of blocks

	void RotateLeft();
	void RotateRight();
	void RotateHalfCircle();

	Vector2Int Measure() const;
	raylib::Rectangle GetBounds() const;

	static Piece* GetMainPiece(MainPieceType mainPieceType);

	Piece()
	{
		pivotOffset = { 0, 0 };
		blockOffsets = nullptr;
		blockColors = nullptr;
		numBlocks = 0;
	}

	Piece(const Piece& piece)
	{
		pivotOffset = piece.pivotOffset;
		blockOffsets = piece.blockOffsets;
		blockColors = piece.blockColors;
		numBlocks = piece.numBlocks;
	}

	Piece(int numBlocks)
	{
		pivotOffset = { 0, 0 };
		blockOffsets = new Vector2Int[numBlocks];
		blockColors = new raylib::Color[numBlocks];
		this->numBlocks = numBlocks;
	}

	Piece(Vector2 pivotOffset, Vector2Int* blockOffsets, raylib::Color* blockColors, int numBlocks)
	{
		this->pivotOffset = pivotOffset;
		this->blockOffsets = blockOffsets;
		this->blockColors = blockColors;
		this->numBlocks = numBlocks;
	}

	~Piece()
	{
		if (blockOffsets != nullptr)
			delete[] blockOffsets;

		if (blockColors != nullptr)
			delete[] blockColors;
	}
};
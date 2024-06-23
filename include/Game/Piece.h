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
	std::vector<Vector2Int> blockOffsets; //block offsets from origin, not pivot
	std::vector<raylib::Color> blockColors; //colors of blocks
	int numBlocks; //amount of blocks

	Piece GetLeftRotation() const;
	Piece GetRightRotation() const;
	Piece GetHalfCircleRotation() const;

	Vector2Int Measure() const;
	raylib::Rectangle GetBounds() const;

	static Piece GetMainPiece(MainPieceType mainPieceType);

	Piece()
	{
		pivotOffset = { 0.0f, 0.0f };
		blockOffsets = std::vector<Vector2Int>(0);;
		blockColors = std::vector<raylib::Color>(0);
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
		pivotOffset = { 0.0f, 0.0f };
		blockOffsets = std::vector<Vector2Int>(numBlocks);;
		blockColors = std::vector<raylib::Color>(numBlocks);
		this->numBlocks = numBlocks;
	}

	Piece(Vector2 pivotOffset, std::vector<Vector2Int> blockOffsets, std::vector<raylib::Color> blockColors, int numBlocks)
	{
		this->pivotOffset = pivotOffset;
		this->blockOffsets = blockOffsets;
		this->blockColors = blockColors;
		this->numBlocks = numBlocks;
	}
};
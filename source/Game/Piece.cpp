#include "Game/Piece.h"

void Piece::RotateLeft()
{
	//90 degrees counter-clockwise rotation
	//implement
}

void Piece::RotateRight()
{
	//90 degrees clockwise rotation
	//implement
}

void Piece::RotateHalfCircle()
{
	//180 degrees rotation, doesn't matter if it's clockwise or counter-clockwise
	for (int i = 0; i < numBlocks; i++)
	{
		blockOffsets[i].x *= -1;
		blockOffsets[i].y *= -1;
	}
}

Vector2Int Piece::Measure() const
{
	int left = INT32_MAX;
	int right = INT32_MIN;

	int top = INT32_MAX;
	int bottom = INT32_MIN;

	for (int i = 0; i < numBlocks; i++)
	{
		if (blockOffsets[i].x < left)
			left = blockOffsets[i].x;
		else if (blockOffsets[i].x > right)
			right = blockOffsets[i].x;

		if (blockOffsets[i].y < top)
			top = blockOffsets[i].y;
		else if (blockOffsets[i].y > bottom)
			bottom = blockOffsets[i].y;
	}

	return { right - left, bottom - top };
}

raylib::Rectangle Piece::GetBounds() const
{
	int left = INT32_MAX;
	int right = INT32_MIN;

	int top = INT32_MAX;
	int bottom = INT32_MIN;

	return { (float)left, (float)top, (float)(right - left), (float)(bottom - top) };
}

Piece Piece::GetMainPiece(MainPieceType mainPieceType)
{
	return Piece();
}
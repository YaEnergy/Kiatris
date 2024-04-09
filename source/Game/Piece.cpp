#include "Game/Piece.h"

void Piece::RotateLeft()
{
	//rotating around pivot
	//90 degrees counter-clockwise rotation
	for (int i = 0; i < numBlocks; i++)
	{
		int newX = (int)(-blockOffsets[i].y + pivotOffset.y + pivotOffset.x);
		int newY = (int)(blockOffsets[i].x - pivotOffset.x + pivotOffset.y);

		blockOffsets[i].x = newX;
		blockOffsets[i].y = newY;
	}
}

void Piece::RotateRight()
{
	//rotating around pivot
	//90 degrees clockwise rotation
	for (int i = 0; i < numBlocks; i++)
	{
		int newX = (int)(blockOffsets[i].y - pivotOffset.y + pivotOffset.x);
		int newY = (int)(-blockOffsets[i].x + pivotOffset.x + pivotOffset.y);

		blockOffsets[i].x = newX;
		blockOffsets[i].y = newY;
	}
}

void Piece::RotateHalfCircle()
{
	//180 degrees rotation, doesn't matter if it's clockwise or counter-clockwise
	//newX - pivotX = -(oldX - pivotX)
	for (int i = 0; i < numBlocks; i++)
	{
		blockOffsets[i].x = -blockOffsets[i].x + (int)(2 * pivotOffset.x);
		blockOffsets[i].y = -blockOffsets[i].y + (int)(2 * pivotOffset.y);
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

Piece* Piece::GetMainPiece(MainPieceType mainPieceType)
{
	const int NUM_MAIN_PIECE_BLOCKS = 4;

	Piece* newPiece = new Piece(NUM_MAIN_PIECE_BLOCKS);

	switch (mainPieceType)
	{
		case PIECE_O:
			newPiece->pivotOffset = { 0.5f, 0.5f };

			for (int i = 0; i < NUM_MAIN_PIECE_BLOCKS; i++)
				newPiece->blockColors[i] = raylib::Color::Yellow();

			newPiece->blockOffsets[0] = { 0, 0 };
			newPiece->blockOffsets[1] = { 1, 0 };
			newPiece->blockOffsets[2] = { 0, 1 };
			newPiece->blockOffsets[3] = { 1, 1 };

			break;
		case PIECE_I:
			newPiece->pivotOffset = { 0.5f, 0.5f };
			for (int i = 0; i < NUM_MAIN_PIECE_BLOCKS; i++)
			{
				newPiece->blockOffsets[i] = { 1, i - 1 };
				newPiece->blockColors[i] = raylib::Color::SkyBlue();
			}
			break;
		case PIECE_S:
			for (int i = 0; i < NUM_MAIN_PIECE_BLOCKS; i++)
				newPiece->blockColors[i] = raylib::Color::Red();

			newPiece->blockOffsets[0] = { 0, 0 };
			newPiece->blockOffsets[1] = { 1, 0 };
			newPiece->blockOffsets[2] = { 0, -1 };
			newPiece->blockOffsets[3] = { -1, -1 };

			break;
		case PIECE_Z:
			for (int i = 0; i < NUM_MAIN_PIECE_BLOCKS; i++)
				newPiece->blockColors[i] = raylib::Color::Green();

			newPiece->blockOffsets[0] = { 0, 0 };
			newPiece->blockOffsets[1] = { -1, 0 };
			newPiece->blockOffsets[2] = { 0, -1 };
			newPiece->blockOffsets[3] = { 1, -1 };

			break;
		case PIECE_L:
			for (int i = 0; i < NUM_MAIN_PIECE_BLOCKS - 1; i++)
			{
				newPiece->blockOffsets[i] = { 0, i - 1 };
				newPiece->blockColors[i] = raylib::Color::Orange();
			}

			newPiece->blockOffsets[NUM_MAIN_PIECE_BLOCKS - 1] = { 1, -1 };
			newPiece->blockColors[NUM_MAIN_PIECE_BLOCKS - 1] = raylib::Color::Orange();
			break;
		case PIECE_J:
			for (int i = 0; i < NUM_MAIN_PIECE_BLOCKS - 1; i++)
			{
				newPiece->blockOffsets[i] = { 0, i - 1 };
				newPiece->blockColors[i] = raylib::Color::Pink();
			}

			newPiece->blockOffsets[NUM_MAIN_PIECE_BLOCKS - 1] = { -1, -1 };
			newPiece->blockColors[NUM_MAIN_PIECE_BLOCKS - 1] = raylib::Color::Pink();
			break;
		case PIECE_T:
			for (int i = 0; i < NUM_MAIN_PIECE_BLOCKS - 1; i++)
			{
				newPiece->blockOffsets[i] = { i - 1, 0 };
				newPiece->blockColors[i] = raylib::Color::Purple();
			}

			newPiece->blockOffsets[NUM_MAIN_PIECE_BLOCKS - 1] = { 0, 1 };
			newPiece->blockColors[NUM_MAIN_PIECE_BLOCKS - 1] = raylib::Color::Purple();
			break;
	}

	return newPiece;
}
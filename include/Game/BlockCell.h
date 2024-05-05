#pragma once

#include "raylib-cpp.hpp"

enum BlockCellState
{
	BLOCK_EMPTY, //No block
	BLOCK_CLEARING, //Clearing this block, for line clear animation
	BLOCK_GRID //A non-empty block, placed or moving
};

struct BlockCell
{
	BlockCellState state;
	raylib::Color color;

	BlockCell()
	{
		state = BLOCK_EMPTY;
		color = raylib::Color::Blank();
	}

	BlockCell(BlockCellState state, raylib::Color color)
	{
		this->state = state;
		this->color = color;
	}
};
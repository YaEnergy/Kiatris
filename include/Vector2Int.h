#pragma once

struct Vector2Int
{
	int x;
	int y;

	Vector2Int(int X, int Y)
	{
		x = X;
		y = Y;
	}

	Vector2Int()
	{
		x = 0;
		y = 0;
	}
};
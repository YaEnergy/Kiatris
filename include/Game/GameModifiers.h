#pragma once

#include "Vector2Int.h"

enum GameMode
{
	GAMEMODE_ENDLESS,
	GAMEMODE_SPRINT
};

struct GameModifiers
{
	GameMode Mode;
	int NumUpAndComingPieces;
	Vector2Int GridSize;
};
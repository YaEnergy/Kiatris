#pragma once

#include "Vector2Int.h"

struct GameOptions
{
	int NumUpAndComingPieces;
	Vector2Int GridSize;
	bool ShowGhostPiece;
	bool EnableStrobingLights;

	GameOptions(int numUpAndComingPieces, Vector2Int gridSize, bool showGhostPiece, bool enableStrobingLights)
	{
		NumUpAndComingPieces = numUpAndComingPieces;
		GridSize = gridSize;
		ShowGhostPiece = showGhostPiece;
		EnableStrobingLights = enableStrobingLights;
	}

	GameOptions()
	{
		NumUpAndComingPieces = 3;
		GridSize = Vector2Int(10, 20);
		ShowGhostPiece = true;
		EnableStrobingLights = true;
	}
};
#pragma once

#include "Vector2Int.h"

struct GameOptions
{
	bool PlayMusic;
	int NumUpAndComingPieces;
	Vector2Int GridSize;
	bool ShowGhostPiece;
	bool EnableStrobingLights;

	GameOptions(bool playMusic, int numUpAndComingPieces, Vector2Int gridSize, bool showGhostPiece, bool enableStrobingLights)
	{
		PlayMusic = playMusic;
		NumUpAndComingPieces = numUpAndComingPieces;
		GridSize = gridSize;
		ShowGhostPiece = showGhostPiece;
		EnableStrobingLights = enableStrobingLights;
	}

	GameOptions()
	{
		PlayMusic = true;
		NumUpAndComingPieces = 3;
		GridSize = Vector2Int(10, 20);
		ShowGhostPiece = true;
		EnableStrobingLights = true;
	}
};
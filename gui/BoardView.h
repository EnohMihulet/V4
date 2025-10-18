#pragma once
#include <vector>

#include <SDL.h>

#include "../chess/GameState.h"
#include "GuiCommon.h"

void drawFenString(GameState& gameState);

void drawBoard(GameState& gameState, GuiState& guiState);

void pieceSelector(GameState& gameState, GuiState& guiState);

void drawBitboards(GameState &gameState, GuiState& guiState);

void drawToggles(GuiState& guiState);

void drawUndoButton(GameState& gameState, GuiState& guiState);

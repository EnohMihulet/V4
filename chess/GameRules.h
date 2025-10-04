#pragma once
#include <vector>

#include "../movegen/MoveGen.h"

bool isCheck(const GameState& gameState, Color color);
bool isDraw(GameResult result);

GameResult getGameResult(GameState& gameState, std::vector<MoveInfo>& history, Color color);


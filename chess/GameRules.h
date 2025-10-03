#pragma once
#include <vector>

#include "../movegen/MoveGen.h"

bool isCheck(const GameState& gameState, Color color);

GameResult getGameResult(GameState& gameState, std::vector<MoveInfo>& history, Color color);


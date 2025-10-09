#pragma once
#include <vector>

#include "../chess/GameState.h"

constexpr uint16 PV_MOVE_SCORE = 10000;
constexpr uint16 TT_MOVE_SCORE = 9000;
constexpr uint16 BASE_CAPTURE_SCORE = 500;
constexpr uint16 CAPTURE_SCORE_MULT = 500;
constexpr uint16 PROMOTION_MULT = 400;

typedef struct PickMoveContext {
	std::vector<uint16> scores;
	Move pvMove;
	Move ttMove;
	uint8 start;
	uint8 size;
} PickMoveContext;

void printMovesAndScores(GameState& gameState);

void scoreMoves(GameState& gameState, std::vector<Move>& moves, PickMoveContext& context);

Move pickMove(std::vector<Move>& moves, PickMoveContext& context);


#pragma once
#include <vector>

#include "../chess/GameState.h"

constexpr uint16 PV_MOVE_SCORE = 10000;
constexpr uint16 TT_MOVE_SCORE = 9000;
constexpr uint16 BASE_CAPTURE_SCORE = 500;
constexpr uint16 CAPTURE_SCORE_MULT = 500;
constexpr uint16 PROMOTION_MULT = 400;

typedef struct ScoreList {
	std::array<uint16, MAX_MOVE_COUNT> list;
	uint16 back = 0;

	inline void clear() { back = 0; }
	inline void push(uint16 score) { assert(back < MAX_MOVE_COUNT); list[back++] = score; }
	inline bool isEmpty() { return back == 0; }
	inline uint16* begin() { return &list[0]; }
	inline uint16* end() { return &list[back]; }
} ScoreList;

typedef struct PickMoveContext {
	ScoreList scores;
	Move pvMove;
	Move ttMove;
	uint16 start;
	uint16 size;
} PickMoveContext;

void printMovesAndScores(GameState& gameState);

void scoreMoves(GameState& gameState, MoveList& moves, PickMoveContext& context);

Move pickMove(MoveList& moves, PickMoveContext& context);


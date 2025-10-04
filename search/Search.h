#pragma once
#include "../chess/GameState.h"

constexpr uint64 TIME_PER_MOVE = 5000;

typedef struct SearchContext {
	uint64 startTime;
	Move bestMoveThisIteration = 0;
	bool searchCanceled;
} SearchContext;

Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history);

int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining);

static inline uint64 cntvct() {
	uint64 cval;
	asm volatile("mrs %0, cntvct_el0" : "=r" (cval));
	return cval;
}

static inline uint64 cntfrq() {
	uint64 freq;
	asm volatile("mrs %0, cntfrq_el0" : "=r" (freq));
	return freq;
}

inline uint64 getTimeElapsed(const SearchContext& context) {
	return (cntvct() - context.startTime) * 1000 / cntfrq();
}

#pragma once

#include "../chess/GameState.h"
#include "../helpers/Timer.h"
#include "../search/TranspositionTable.h"

constexpr uint64 TIME_PER_MOVE = 5000;

typedef struct SearchContext {
	uint64 startTime;
	Move bestMoveThisIteration = 0;
	bool searchCanceled;
} SearchContext;

Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history);

#ifdef DEBUG_MODE
typedef struct SearchStats {
	uint64 nodes = 0;
	uint64 prunedNodes = 0;
	uint64 betaCutOffs = 0;

	uint64 ttProbes = 0;
	uint64 ttHits = 0;
	uint64 ttHitsUseful = 0;
	uint64 ttHitCutoffs = 0;

	uint64 ttStores = 0;
	uint64 ttStoresExact = 0;
	uint64 ttStoresLower = 0;
	uint64 ttStoresUpper = 0;
} SearchStats;

typedef struct SearchTimes {
	double total;

	double evaluation;
	double transpositionLookUp;
	double gameStateCheck;

	double moveGeneration;
	double moveFiltering;
	double moveScoring;
	double movePicking;

	double moveMaking;
	double moveUnmaking;
} SearchTimes;

int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, 
			  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining, SearchStats& stats, SearchTimes& times);
#else
int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, 
			  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining);
#endif

void clearTranspositionTable();

#pragma once

#include "../chess/GameState.h"
#include "../search/MoveSorter.h"

constexpr uint64 TIME_PER_MOVE = 5000;
constexpr uint64 MAX_PLY = 30;

typedef struct SearchContext {
	uint64 startTime;
	Move bestMoveThisIteration = 0;
	bool searchCanceled;
} SearchContext;

Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history);

enum MoveBucket : uint8 {
	B_TT, B_PV, B_Promo, B_GoodCap, B_Killer1, B_Counter, B_Killer2, B_QuietHist, B_BadCap, B_Other, B_Count
};

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

	// uint64 plyNodes[MAX_PLY] = {};
	// uint64 legalMoves[MAX_PLY] = {};
	// uint64 cutoffCount[MAX_PLY] = {};
	// uint64 cutoffIndexSum[MAX_PLY] = {};
	// uint64 firstMoveCutoffs[MAX_PLY] = {};
} SearchStats;

typedef struct SearchTimes {
	uint64 total;

	uint64 transpositionLookUp;
	uint64 transpositionInsertion;

	uint64 evaluation;
	uint64 gameResultCheck;
	uint64 pickContextSetup;

	uint64 moveGeneration;
	uint64 moveScoring;
	uint64 movePicking;

	uint64 moveMaking;
	uint64 moveUnmaking;

	uint64 repetitionPush;
	uint64 repetitionPop;
} SearchTimes;

typedef struct MovePool {
	std::array<MoveList, MAX_PLY> pool;

	MoveList& getMoveList(uint8 depth) {
		pool[depth].clear();
		return pool[depth];
	}
} MovePool;

typedef struct MoveScorePool {
	std::array<ScoreList, MAX_PLY> pool;

	ScoreList& getScoreList(uint8 depth) {
		pool[depth].clear();
		return pool[depth];
	}

} MoveScorePool;

typedef struct QuiescencePool {
	std::array<MoveList, 5> pool;

	MoveList& getMoveList(uint8 depth) {
		pool[depth].clear();
		return pool[depth];
	}
} QuiescencePool;

// Debug version
int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, 
			  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining, SearchStats& stats, SearchTimes& times);

// Release version
int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, 
			  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining);

void clearTranspositionTable();

#pragma once
#include <vector>

#include "../chess/GameState.h"
#include "Common.h"
#include "Move.h"

constexpr uint16 PV_MOVE_SCORE   = 65000;
constexpr uint16 TT_MOVE_SCORE   = 60000;

constexpr uint16 PROMOTION_BASE  = 56000;
constexpr uint16 PROMO_STEP      = 256;

constexpr uint16 GOOD_CAPTURE_BASE    = 50000;
constexpr uint16 MVV_WEIGHT      = 32;

constexpr uint16 KILLER_MOVE_1_SCORE  = 42000;
constexpr uint16 COUNTER_MOVE_SCORE   = 41000;
constexpr uint16 FOLLOW_UP_MOVE_SCORE  = 40000;
constexpr uint16 KILLER_MOVE_2_SCORE  = 39000;

constexpr uint16 QUIET_BASE = 30000;
constexpr int16 MAX_HISTORY_BONUS = 8000;

constexpr uint16 BAD_CAPTURE_BASE = 20000;

constexpr uint16 LOWEST_BASE = 10000;

constexpr uint8 MOVE_TABLE_SIZE = 30;
constexpr uint8 CONTINUATION_SIZE = 4;

typedef struct ScoreList {
	std::array<uint16, MAX_MOVE_COUNT> list;
	uint16 back = 0;

	inline void clear() { back = 0; }
	inline void push(uint16 score) { assert(back < MAX_MOVE_COUNT); list[back++] = score; }
	inline bool isEmpty() { return back == 0; }
	inline uint16* begin() { return &list[0]; }
	inline uint16* end() { return &list[back]; }
} ScoreList;

typedef struct MTEntry {
	Move move1;
	Move move2;
} MTEntry;

typedef struct PickMoveContext {
	ScoreList& scores;
	Move pvMove;
	Move ttMove;
	MTEntry killerMoves;
	uint16 start;
	uint16 size;
} PickMoveContext;

typedef struct MoveTable {
	std::array<MTEntry, MOVE_TABLE_SIZE> table;
	MoveTable() { clearTable(); }

	inline void clearTable() { for (uint8 i = 0; i < MOVE_TABLE_SIZE; i++) table[i] = {NULL_MOVE, NULL_MOVE}; }

	inline MTEntry& getEntry(uint8 pliesFromRoot) { return table[pliesFromRoot]; }
	
	inline void storeEntry(uint8 pliesFromRoot, Move move) { 
		MTEntry& e = table[pliesFromRoot];
		if (e.move1.val != move.val) {
			e.move2 = e.move1;
			e.move1 = move;
		}
	}	
} MoveTable;

typedef struct HistoryTable {
	std::array<std::array<std::array<int16, 64>, 64>, 2> table;

	HistoryTable() { clearTable(); }

	inline void clearTable() { 
		for (uint32 from = 0; from < 64; from++) {
			for (uint8 to = 0; to < 64; to++) {
				table[0][from][to] = 0;
				table[1][from][to] = 0;
			}
		}
	}

	inline void update(Color c, uint8 from, uint8 to, int16 bonus) { 
		int16 clampedBonus = bonus < -MAX_HISTORY_BONUS ? -MAX_HISTORY_BONUS : bonus > MAX_HISTORY_BONUS ? MAX_HISTORY_BONUS : bonus;
		table[c][from][to] += clampedBonus - table[c][from][to] * abs(clampedBonus) / MAX_HISTORY_BONUS;
	}

} HistoryTable;

typedef struct CounterMoveTable {
	std::array<std::array<Move, 64>, PIECE_COUNT> table;

	CounterMoveTable() { clearTable(); }

	inline void clearTable() {
		for (uint32 p = 0; p < PIECE_COUNT; p++) {
			for (uint8 to = 0; to < 64; to++) {
				table[p][to] = NULL_MOVE;
			}
		}
	}

	inline void addMove(GameState& s, Move m, std::vector<Move>& mStack) {
		if (mStack.size() < 1) return;
		Move prevMove = mStack[mStack.size() - 1];
		table[s.pieceAt(prevMove.getStartSquare())][prevMove.getTargetSquare()] = m;
	}

	inline Move getMove(GameState& s, std::vector<Move>& mStack) { 
		if (mStack.size() < 1) return NULL_MOVE;
		Move prevMove = mStack[mStack.size() - 1];
		return table[s.pieceAt(prevMove.getStartSquare())][prevMove.getTargetSquare()];
	}
} CounterMoveTable;

typedef struct FollowUpMoveTable{
	std::array<std::array<Move, 64>, PIECE_COUNT> table;

	FollowUpMoveTable() { clearTable(); }

	inline void clearTable() {
		for (uint32 p = 0; p < PIECE_COUNT; p++) {
			for (uint8 to = 0; to < 64; to++) {
				table[p][to] = NULL_MOVE;
			}
		}
	}

	inline void addMove(GameState& s, Move m, std::vector<Move>& mStack) {
		if (mStack.size() < 2) return;
		Move prevMove = mStack[mStack.size() - 2];
		table[s.pieceAt(prevMove.getStartSquare())][prevMove.getTargetSquare()] = m;
	}

	inline Move getMove(GameState& s, std::vector<Move>& mStack) { 
		if (mStack.size() < 2) return NULL_MOVE;
		Move prevMove = mStack[mStack.size() - 2];
		return table[s.pieceAt(prevMove.getStartSquare())][prevMove.getTargetSquare()];
	}
} FollowUpMoveTable;

// Not worth it. Only few hundred cut offs despite tens of thousands of attempts.
// Maybe implement a continuation history table?
// typedef struct ContinuationTable {
// 	std::array<std::array<std::array<Move, 64>, PIECE_COUNT>, CONTINUATION_SIZE> table;
// 
// 	ContinuationTable() { clearTable(); }
// 
// 	inline void clearTable() {
// 		for (uint32 p = 0; p < PIECE_COUNT; p++) {
// 			for (uint8 to = 0; to < 64; to++) {
// 				for (uint ply = 0; ply < CONTINUATION_SIZE; ply++)
// 					table[ply][p][to] = NULL_MOVE;
// 			}
// 		}
// 	}
// 
// 	inline void addMove(GameState& s, Move m, std::vector<Move>& mStack) {
// 		uint8 stackSize = mStack.size();
// 		uint8 ply = 0;
// 		while (stackSize != 0 && ply < CONTINUATION_SIZE) {
// 			Move prevMove = mStack[mStack.size() - ply - 1];
// 			table[ply][s.pieceAt(prevMove.getStartSquare())][prevMove.getTargetSquare()] = m;
// 			ply++;
// 		}
// 	}
// 
// 	inline Move getMove(GameState& s, uint8 ply, std::vector<Move>& mStack) { 
// 		if (mStack.size() > ply && ply < CONTINUATION_SIZE) {
// 			Move prevMove = mStack[mStack.size() - ply - 1];
// 			return table[ply][s.pieceAt(prevMove.getStartSquare())][prevMove.getTargetSquare()];
// 		}
// 		return NULL_MOVE;
// 	}
// } ContinuationTable;

void printMovesAndScores(GameState& gameState);

void scoreMoves(GameState& gameState, MoveList& moves, PickMoveContext& context, HistoryTable& historyTable, CounterMoveTable& counterTable, FollowUpMoveTable& followUpTable, std::vector<Move>& moveStack);

Move pickMove(MoveList& moves, PickMoveContext& context);


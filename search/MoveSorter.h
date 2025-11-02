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

constexpr uint16 KILLER_MOVE_1_SCORE   = 42000;
constexpr uint16 COUNTER_MOVE_SCORE    = 41500;
constexpr uint16 FOLLOW_UP_MOVE_SCORE  = 41000;
constexpr uint16 KILLER_MOVE_2_SCORE   = 40000;

constexpr uint16 MAX_HISTORY_SCORE = 37500;
constexpr uint16 HIGH_HISTORY_SCORE = 34750;
constexpr uint16 QUIET_BASE = 30000;
constexpr uint16 MAX_TOTAL_BONUS    = 7500;
constexpr int16 MAX_HISTORY_BONUS   = 1500;
constexpr int16 MAX_COUNTER_BONUS   = 3500;
constexpr int16 MAX_FOLLOW_UP_BONUS = 2500;

constexpr uint16 BAD_CAPTURE_BASE = 20000;

constexpr uint16 LOWEST_BASE = 10000;

constexpr uint8 MOVE_TABLE_SIZE   = 30;
constexpr uint8 CONTINUATION_SIZE = 4;


typedef struct ContEntry {
	Piece p;
	uint8 to;
} ContEntry;

typedef struct ContinuationStack {
	std::vector<ContEntry> stack;

	ContinuationStack() { stack.reserve(256); }

	void push(Piece p, uint8 to) { stack.push_back({p, to}); }
	void push(GameState& s, Move m) { stack.push_back({s.pieceAt(m.getStartSquare()), (uint8)m.getTargetSquare()}); }

	void pop() { stack.pop_back(); }

	int8 at(uint8 lag, ContEntry& e) { 
		uint8 s = stack.size();
		if (lag >= s) return -1;
		e = stack[stack.size() - 1 - lag]; 
		return 0;
	}

} ContinuationStack;

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
		for (uint8 from = 0; from < 64; from++) {
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

	inline int16 getScore(Color c, uint8 from, uint8 to) {
		return table[c][from][to];
	}

} HistoryTable;

typedef struct CounterMoveTable {
	std::array<std::array<Move, 64>, PIECE_COUNT> table;

	CounterMoveTable() { clearTable(); }

	inline void clearTable() {
		for (uint8 p = 0; p < PIECE_COUNT; p++) {
			for (uint8 to = 0; to < 64; to++) {
				table[p][to] = NULL_MOVE;
			}
		}
	}

	inline void addMove(Move m, ContinuationStack& cS) {
		ContEntry e;
		if (cS.at(0, e) < 0) return;
		table[e.p][e.to] = m;
	}

	inline Move getMove(ContinuationStack& cS) { 
		ContEntry e;
		if (cS.at(0, e) < 0) return NULL_MOVE;
		return table[e.p][e.to];
	}

} CounterMoveTable;

typedef struct FollowUpMoveTable{
	std::array<std::array<Move, 64>, PIECE_COUNT> table;

	FollowUpMoveTable() { clearTable(); }

	inline void clearTable() {
		for (uint8 p = 0; p < PIECE_COUNT; p++) {
			for (uint8 to = 0; to < 64; to++) {
				table[p][to] = NULL_MOVE;
			}
		}
	}

	inline void addMove(Move m, ContinuationStack& cS) {
		ContEntry e;
		if (cS.at(1, e) < 0) return;
		table[e.p][e.to] = m;
	}

	inline Move getMove(ContinuationStack& cS) { 
		ContEntry e;
		if (cS.at(1, e) < 0) return NULL_MOVE;
		return table[e.p][e.to];
	}

} FollowUpMoveTable;

typedef struct CounterHistoryTable{
	std::array<std::array<std::array<std::array<int16, 64>, 12>, 64>, 12> table;

	CounterHistoryTable() { clearTable(); }

	inline void clearTable() { 
		for (uint8 p1 = 0; p1 < PIECE_COUNT; p1++) {
			for (uint8 to1 = 0; to1 < 64; to1++) {
				for (uint8 p2 = 0; p2 < PIECE_COUNT; p2++) {
					for (uint8 to2 = 0; to2 < 64; to2++) {
						table[p1][to1][p2][to2] = 0;
					}
				}
			}
		}
	}

	inline void update(Piece p, uint8 to, int16 bonus, ContinuationStack& cS) { 
		ContEntry e;
		if (cS.at(0, e) < 0) return;
		int16 clampedBonus = bonus < -MAX_COUNTER_BONUS ? -MAX_COUNTER_BONUS : bonus > MAX_COUNTER_BONUS ? MAX_COUNTER_BONUS : bonus;
		table[e.p][e.to][p][to] += clampedBonus - table[e.p][e.to][p][to] * abs(clampedBonus) / MAX_COUNTER_BONUS;
	}

	inline int16 getScore(ContEntry e, Piece p, uint8 to) {
		return table[e.p][e.to][p][to];
	}

	inline int16 getScore(Piece p, uint8 to, ContinuationStack cs) {
		ContEntry e;
		if (cs.at(1, e) < 0) return 0;
		return table[e.p][e.to][p][to];
	}

} CounterHistoryTable;

typedef struct FollowUpHistoryTable{
	std::array<std::array<std::array<std::array<int16, 64>, 12>, 64>, 12> table;

	FollowUpHistoryTable() { clearTable(); }

	inline void clearTable() { 
		for (uint8 p1 = 0; p1 < PIECE_COUNT; p1++) {
			for (uint8 to1 = 0; to1 < 64; to1++) {
				for (uint8 p2 = 0; p2 < PIECE_COUNT; p2++) {
					for (uint8 to2 = 0; to2 < 64; to2++) {
						table[p1][to1][p2][to2] = 0;
					}
				}
			}
		}
	}

	inline void update(Piece p, uint8 to, int16 bonus, ContinuationStack& cS) { 
		ContEntry e;
		if (cS.at(1, e) < 0) return;
		int16 clampedBonus = bonus < -MAX_FOLLOW_UP_BONUS ? -MAX_FOLLOW_UP_BONUS : bonus > MAX_FOLLOW_UP_BONUS ? MAX_FOLLOW_UP_BONUS : bonus;
		table[e.p][e.to][p][to] += clampedBonus - table[e.p][e.to][p][to] * abs(clampedBonus) / MAX_FOLLOW_UP_BONUS;
	}

	inline int16 getScore(ContEntry e, Piece p, uint8 to) {
		return table[e.p][e.to][p][to];
	}

	inline int16 getScore(Piece p, uint8 to, ContinuationStack cs) {
		ContEntry e;
		if (cs.at(1, e) < 0) return 0;
		return table[e.p][e.to][p][to];
	}

} FollowUpHistoryTable;

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

void scoreMoves(GameState& gameState, MoveList& moves, PickMoveContext& context, HistoryTable& historyTable, CounterHistoryTable& cHistoryTable, FollowUpHistoryTable& fHistoryTable,
		CounterMoveTable& counterTable, FollowUpMoveTable& followUpTable, ContinuationStack& moveStack);

Move pickMove(MoveList& moves, PickMoveContext& context);


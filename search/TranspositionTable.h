#pragma once

#include "../chess/Common.h"
#include "../chess/Move.h"

enum NodeType { Exact, UpperBound, LowerBound };

constexpr uint32 TABLE_SIZE = 524288;
constexpr int16 SCORE_SENTINAL = -3103;

struct Entry {
	uint64 zobrist;
	Move bestMove;
	int16 score;
	uint8 depth;
	NodeType nodeType;
};

struct TranspositionTable {
	Entry* table;

	TranspositionTable() { 
		table = new Entry[TABLE_SIZE]{}; 
		clear_table();
	}

	inline void clear_table() {
		for (uint32 i = 0; i < TABLE_SIZE; i++) table[i] = {0,NULL_MOVE,SCORE_SENTINAL,0,Exact};
	}

	inline uint32 index(uint64 zobrist) const {
		return (zobrist ^ (zobrist >> 32)) & (TABLE_SIZE - 1); // Or just zobrist & (TABLE_SIZE - 1) probably little to no difference
	}

	inline void storeEntry(const Entry& entry) {
		uint32 i = index(entry.zobrist);
		if (table[i].zobrist != 0 && table[i].depth > entry.depth)
			return;
		table[i] = entry;
	}

	inline int16 getEval(uint64 zobrist) const {
		const Entry& entry = table[index(zobrist)];
		return (entry.zobrist == zobrist) ? entry.score : SCORE_SENTINAL;
	}

	inline NodeType getNodeType(int16 alpha, int16 beta, int16 originalAlpha) const {
		if (alpha >= beta) return LowerBound;
		else if (alpha <= originalAlpha) return UpperBound;
		return Exact;
	}
};

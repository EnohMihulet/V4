#pragma once
#include <vector>

#include "../movegen/MoveGen.h"


typedef struct RepetitionTable {
	static constexpr uint16 MAX_ENTRIES = 512;
	uint64 keys[MAX_ENTRIES];
	uint64 counts[MAX_ENTRIES];
	uint16 size = 0;

	void clear() { size = 0; }

	void push(uint64 key) {
		assert(size < 512);
		if (size == 0) return;
		for (uint16 i = size - 1; i >= 0; i--) {
			if (keys[i] == key) {
				counts[i]++;
				return;
			}
		}
		keys[size] = key;
		counts[size] = 1;
		size++;
	}

	void pop(uint64 key) {
		if (size == 0) return;
		for (uint16 i = size - 1; i >= 0; i--) {
			if (keys[i] == key) {
				if (--counts[i] == 0) {
					keys[i] = keys[size-1];
					counts[i] = counts[size-1];
					size--;
				}
				return;
			}
		}
	}

	bool isRepeated(uint64 key) {
		for (uint16 i = 0; i < size; i++) {
			if (keys[i] == key) return counts[i] >= 2;
		}
		return false;
	}

} RepetitionTable;

bool isInsufficientMaterial(const GameState& gameState);

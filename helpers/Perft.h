#pragma once

#include "../chess/GameState.h"

struct PerftStats {
	uint64 nodes = 0;
	uint64 captures = 0;
	uint64 enPassants  = 0;
	uint64 castles = 0;
	uint64 ksCastles = 0;
	uint64 qsCastles = 0;
	uint64 promotions = 0;
	uint64 dblPushes = 0;
	uint64 checkmates = 0;
	uint64 stalemates = 0;
};

void runPerftTest();


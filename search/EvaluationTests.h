#pragma once

#include "../chess/GameState.h"

constexpr std::string_view FIVE_MIRRORED_FENS[5] = { "rnbq1rk1/ppp1ppbp/5np1/3p4/3P4/5NP1/PPP1PPBP/RNBQ1RK1 w - - 2 6",
		"2kr1b1r/pp1bpppp/n1pq1n2/3p4/3P4/N1PQ1N2/PP1BPPPP/2KR1B1R w - - 0 1",
		"1r3rk1/p2bpp1p/3bn1p1/8/8/3BN1P1/P2BPP1P/1R3RK1 w - - 0 1",
		"7r/pp1k1p1p/4pn2/2b5/2B5/4PN2/PP1K1P1P/7R w - - 0 1",
		"8/1p3p2/2k1p3/3n4/3N4/2K1P3/1P3P2/8 w - - 0 1" };

void testIsolatePawn();
void testPassedPawn();
void testKnightPair();
void testKingSafety();
void testDoubledPawns();
void testStartingPosition();
void testEqualPositions();

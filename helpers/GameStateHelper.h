#pragma once

#include "../chess/GameState.h"

#define PASS(msg) std::cout << "PASS: " << msg << std::endl
#define FAIL(msg) std::cout << "FAIL: " << msg << std::endl

void printBitboard(Bitboard bb);
void printGameStateBitboards(const GameState& state);
void printBoard(const GameState& state);

bool gameStatesAreEqual(const GameState& state1, const GameState& state2);

void testMakeUnmake(GameState& startState, const Move& move, const std::string& description);
void testMakeUnmakeMove();
void testAllMoves(GameState state);

void testMakeUnmakePawns();
void testMakeUnmakeKnights();
void testMakeUnmakeBishops();
void testMakeUnmakeRooks();
void testMakeUnmakeQueens();
void testMakeUnmakeKings();

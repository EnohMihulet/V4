#pragma once

#include "../chess/GameState.h"

constexpr int16 PIECE_VALUES[6] = {100, 320, 330, 500, 900, 20000};

constexpr int16 KNIGHT_ADJUSTMENT[9] = {-20, -16, -12, -8, -4,  0,  4,  8, 12};
constexpr int16 ROOK_ADJUSTMENT[9] = {15,  12,   9,  6,  3,  0, -3, -6, -9};

constexpr int16 BISHOP_PAIR = 40;
constexpr int16 KNIGHT_PAIR = -10;
constexpr int16 ROOK_PAIR = 25;

constexpr int16 PASSED_PAWNS[7] = {5, 10, 15, 20, 35, 60, 100};
constexpr int16 CONNECTED_PAST_PAWNS = 30;
constexpr int16 DOUBLED_PAWNS = -5;
constexpr int16 ISOLATED_PAWNS = -15;
constexpr int16 BACKWARD_PAWN = -10;

constexpr int16 ROOK_OPEN_FILE = 20;
constexpr int16 ROOK_SEMI_OPEN_FILE = 10;

constexpr int16 CASTLED = 15;
constexpr int16 STRONG_PAWN_SHIELD = 15;
constexpr int16 MID_PAWN_SHIELD = 10;
constexpr int16 EXPOSED_KING = -40;

constexpr int16 KNIGHT_MOBILITY_BONUS = 4;
constexpr int16 BISHOP_MOBILITY_BONUS = 3;
constexpr int16 ROOK_MOBILITY_BONUS = 2;
constexpr int16 QUEEN_MOBILITY_BONUS = 1;

constexpr uint16 TOTAL_PHASE = 24;

typedef struct EvalComponents {
	int pawns;
	int knights;
	int bishops;
	int rooks;
	int queens;
	int kings;
} EvalComponents;

int16 evaluate(const GameState& gameState, Color color);
int16 evaluate(const GameState& gameState);

int16 evaluatePawns(const GameState& gameState, float mgFactor, float egFactor);

int16 evaluateKnights(const GameState& gameState, float mgFactor, float egFactor);

int16 evaluateBishops(const GameState& gameState, float mgFactor, float egFactor);

int16 evaluateRooks(const GameState& gameState, float mgFactor, float egFactor);

int16 evaluateQueen(const GameState& gameState, float mgFactor, float egFactor);

int16 evaluateKing(const GameState& gameState, float mgFactor, float egFactor);

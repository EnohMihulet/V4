#pragma once

#include <array>
#include <vector>

#include "../chess/GameState.h"

bool isSquareAttacked(const GameState& gameState, uint64 pos, Color color);

void filterMoves(GameState& gameState, std::vector<MoveInfo>& history, std::vector<Move>& moves, Color color);

void generatePawnMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking);

void generateKnightMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking);

void generateBishopMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking);

void generateRookMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking);

void generateQueenMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking);

void generateKingMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking);

void generateAllMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking);

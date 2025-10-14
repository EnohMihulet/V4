#pragma once

#include <array>
#include <vector>

#include "../chess/GameState.h"

bool isSquareAttacked(const GameState& gameState, uint64 pos, Color color);

void filterMoves(GameState& gameState, std::vector<MoveInfo>& history, std::vector<Move>& moves, Color color);

Bitboard getPossibleBishopAttackers(uint8 square, Bitboard occupied);

Bitboard getPossibleRookAttackers(uint8 square, Bitboard occupied);

void computeCheckAndPinMasks(const GameState& gameState, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generatePawnMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generateKnightMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generateBishopMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generateRookMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generateQueenMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generateKingMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generateAllMoves(GameState& gameState, std::vector<Move>& moves, Color us);

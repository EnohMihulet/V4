#pragma once

#include <array>
#include <vector>

#include "../chess/GameState.h"

bool isSquareAttacked(const GameState& gameState, uint64 pos, Color color);

Bitboard getPossibleBishopAttackers(uint8 square, Bitboard occupied);

Bitboard getPossibleRookAttackers(uint8 square, Bitboard occupied);

void computeCheckAndPinMasks(const GameState& gameState, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generatePawnMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);
void generateKnightMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces);
void generateBishopMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);
void generateRookMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);
void generateQueenMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);
void generateKingMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask);
void generateAllMoves(GameState& gameState, MoveList& moves, Color us);
void generateAllMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);

void generatePawnCaptureMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);
void generateKnightCaptureMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces);
void generateBishopCaptureMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);
void generateRookCaptureMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);
void generateQueenCaptureMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);
void generateKingCaptureMoves(GameState& gameState, MoveList& moves, Color us);
void generateAllCaptureMoves(GameState& gameState, MoveList& moves, Color us);
void generateAllCaptureMoves(GameState& gameState, MoveList& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays);


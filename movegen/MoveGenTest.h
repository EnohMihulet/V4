#pragma once
#include <chrono>
#include <iostream>
#include <vector>

#include "MoveGen.h"


void printPieceMoves(GameState& gameState, Piece piece);

void printGeneratedMoves(GameState& gameState, Color color);

void timePieceMoves(GameState& gameState, Piece piece);

void timeMoves(GameState& gameState, Color color);

void testPawnMoveGeneration();
void testKnightMoveGeneration();
void testBishopMoveGeneration();
void testRookMoveGeneration();
void testQueenMoveGeneration();
void testKingMoveGeneration();

#pragma once
#include <chrono>
#include <iostream>
#include <vector>

#include "MoveGen.h"


void printPieceMoves(GameState& gameState, Piece piece);

void printPsuedoLegalMoves(GameState& gameState, Color color);

void printLegalMoves(GameState& gameState, Color color);

void timePsuedoLegalPieceMoves(GameState& gameState, Piece piece);

void timeLegalPieceMoves(GameState& gameState, Piece piece);

void timePsuedoLegalMoves(GameState& gameState, Color color);

void timeLegalMoves(GameState& gameState, Color color);

void timeMoveFiltering(GameState& gameState, Color color, std::vector<Move> moves=std::vector<Move>());

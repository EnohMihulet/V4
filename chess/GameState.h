#pragma once
#include <array>
#include <string>
#include <vector>

#include "Move.h"


typedef struct GameState {

	std::array<Piece, 64> board;
	std::array<Bitboard, 15> bitboards;
	uint64 zobristHash;
	uint8 castlingRights;
	uint8 enPassantFile;
	uint8 halfMoves;
	uint8 fullMoves;
	Color colorToMove;

	GameState(const std::string& fen);

	void makeMove(Move move, std::vector<MoveInfo>& history);
	void unmakeMove(Move move, std::vector<MoveInfo>& history);

	void setPiece(uint16 square, Piece piece);
	void clearSquare(uint16 square);
	Piece pieceAt(uint16 sq) const;

	bool isCheck(Color color);
	GameResult getGameResult(std::vector<MoveInfo>& history, Color color);
} GameState;

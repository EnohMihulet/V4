#pragma once
#include <array>
#include <string>
#include <vector>

#include "Common.h"
#include "Move.h"
#include "../helpers/Zobrist.h"

typedef struct GameState {

	std::array<Piece, 64> board;
	std::array<Bitboard, 15> bitboards;
	uint64 zobristHash;
	uint8 castlingRights;
	uint8 enPassantFile;
	uint8 halfMoves;
	uint8 fullMoves;
	Color colorToMove;

	GameState();
	GameState(const std::string& fen);

	void setPosition(const std::string& fen);
	void makeMove(Move move, std::vector<MoveInfo>& history);
	void unmakeMove(Move move, std::vector<MoveInfo>& history);

	void setPiece(uint16 square, Piece piece);
	void clearSquare(uint16 square);
	Piece pieceAt(uint16 sq) const;

	bool isCheck(Color color);
	GameResult getGameResult(std::vector<MoveInfo>& history, Color color);
	bool isEnPassantCaptureLegal(uint16 enPassantFile, Color color) const;
} GameState;



constexpr std::array<uint8, 64> makeCastlingRightsMask() {
	std::array<uint8, 64> t{};
	for (auto& v : t) v = W_KING_SIDE | W_QUEEN_SIDE | B_KING_SIDE | B_QUEEN_SIDE;
	t[0] = B_KING_SIDE | B_QUEEN_SIDE | W_KING_SIDE;
	t[4] = B_KING_SIDE | B_QUEEN_SIDE;
	t[7] = B_KING_SIDE | B_QUEEN_SIDE | W_QUEEN_SIDE; 
	t[56] = W_KING_SIDE | W_QUEEN_SIDE | B_KING_SIDE;
	t[60] = W_KING_SIDE | W_QUEEN_SIDE;
	t[63] = W_KING_SIDE | W_QUEEN_SIDE | B_QUEEN_SIDE;
	return t;
}; 

constexpr std::array<bool, 14> makeSimpleMoveArr() {
	std::array<bool, 14> t{};
	for (auto& v : t) v = false;
	t[0] = true;
	t[1] = true;
	t[2] = true;
	return t;
}

inline constexpr auto CASTLING_RIGHTS_MASK = makeCastlingRightsMask();
inline constexpr auto IS_SIMPLE_MOVE = makeSimpleMoveArr();

inline void makeWKingSide(GameState& g) {
	g.castlingRights &= (B_KING_SIDE | B_QUEEN_SIDE); 
	g.clearSquare(7); 
	g.setPiece(5, WRook); 
	g.zobristHash ^= PIECE_ZOBRIST_KEYS[WRook*64 + 7];
	g.zobristHash ^= PIECE_ZOBRIST_KEYS[WRook*64 + 5];
}
inline void makeBKingSide(GameState& g) { 
	g.castlingRights &= (W_KING_SIDE | W_QUEEN_SIDE); 
	g.clearSquare(63); 
	g.setPiece(61, BRook); 
	g.zobristHash ^= PIECE_ZOBRIST_KEYS[BRook*64 + 63];
	g.zobristHash ^= PIECE_ZOBRIST_KEYS[BRook*64 + 63];
}
inline void makeWQueenSide(GameState& g) { 
	g.castlingRights &= (B_KING_SIDE | B_QUEEN_SIDE); 
	g.clearSquare(0); 
	g.setPiece(3, WRook);
	g.zobristHash ^= PIECE_ZOBRIST_KEYS[WRook*64 + 0];
	g.zobristHash ^= PIECE_ZOBRIST_KEYS[WRook*64 + 3];
}
inline void makeBQueenSide(GameState& g) { 
	g.castlingRights &= (W_KING_SIDE | W_QUEEN_SIDE); 
	g.clearSquare(56); 
	g.setPiece(59, BRook);
	g.zobristHash ^= PIECE_ZOBRIST_KEYS[BRook*64 + 56];
	g.zobristHash ^= PIECE_ZOBRIST_KEYS[BRook*64 + 56];
}

inline void undoWKingSide(GameState& g) { g.clearSquare(5); g.setPiece(7, WRook); g.clearSquare(6); g.setPiece(4, WKing); }
inline void undoBKingSide(GameState& g) { g.clearSquare(61); g.setPiece(63, BRook); g.clearSquare(62); g.setPiece(60, BKing); }
inline void undoWQueenSide(GameState& g) { g.clearSquare(3); g.setPiece(0, WRook); g.clearSquare(2); g.setPiece(4, WKing); }
inline void undoBQueenSide(GameState& g) { g.clearSquare(59); g.setPiece(56, BRook); g.clearSquare(58); g.setPiece(60, BKing); }

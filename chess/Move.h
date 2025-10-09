#pragma once
#include <array>
#include <cstdint>
#include <cassert>
#include <string>

#include "Common.h"

#ifdef DEBUG_MODE
typedef struct MoveInfo {
	uint64 zobristHash;
	uint8 castlingRights;
	uint8 enPassantFile;
	uint8 halfMoves;
	Piece capturedPiece;
	std::array<Bitboard, 15> bitboards;
} MoveInfo;
#else
typedef struct MoveInfo {
	uint64 zobristHash;
	uint8 castlingRights;
	uint8 enPassantFile;
	uint8 halfMoves;
	Piece capturedPiece;
} MoveInfo;

#endif

typedef struct Move {
	
	uint16 val; // Flags - Target Position - Start Position  0000 - 000000 - 000000 
	

	constexpr Move(uint16 v = 0) : val(v) {  }
	Move(uint16 startSq, uint16 targetSq, uint16 FLAGS) {
		assert(startSq <= 63 && targetSq <= 63 && FLAGS <= 15);
		val = startSq | (targetSq << 6) | (FLAGS << 12);
	}
	Move(const std::string& uicMove);

	inline bool isNull() {
		return val == 0;
	}
	
	inline uint16 getStartSquare() const { return val & START_SQUARE_MASK; }
	inline uint16 getTargetSquare() const { return (val & TARGET_SQUARE_MASK) >> 6; }
	
	inline uint64 getStartPos() const { return (1ULL << getStartSquare()); }
	inline uint64 getTargetPos() const { return (1ULL << getTargetSquare()); }
	
	inline uint16 getFlags() const { return (val & FLAG_MASK) >> 12; }
	
	inline bool isCapture() const { return (getFlags() & 1ULL) == 1ULL; }
	inline bool isEnPassant() const { return getFlags() == EN_PASSANT_FLAG; }
	inline bool isKingSideCastle() const { return getFlags() == KING_SIDE_FLAG; }
	inline bool isQueenSideCastle() const { return getFlags() == QUEEN_SIDE_FLAG; }
	inline bool isTwoUpMove() const { return getFlags() == PAWN_TWO_UP_FLAG; }
	
	inline bool isKnightPromotion() const { return (getFlags() & 0b1110) == KNIGHT_PROMOTE_FLAG; }
	inline bool isBishopPromotion() const { return (getFlags() & 0b1110) == BISHOP_PROMOTE_FLAG; }
	inline bool isRookPromotion() const { return (getFlags() & 0b1110) == ROOK_PROMOTE_FLAG; } 
	inline bool isQueenPromotion() const { return (getFlags() & 0b1110) == QUEEN_PROMOTE_FLAG; }
	inline bool isPromotion() const { return getFlags() >= KNIGHT_PROMOTE_FLAG; }
	std::string moveToString() const;
} Move;
static_assert(sizeof(Move) == 2);

constexpr Move NULL_MOVE{};

inline std::string squareToString(uint16 sq) {
	char file = 'a' + (sq % 8);
	char rank = '1' + (sq / 8);
	return std::string() + file + rank;
}



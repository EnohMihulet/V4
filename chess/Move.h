#pragma once
#include <cstdint>
#include <cassert>

#include "Common.h"

typedef struct MoveInfo {
	uint64 zobristHash;
	uint8 castlingRights;
	uint8 enPassantFile;
	uint8 halfMoves;
	Piece capturedPiece;
} MoveInfo;

typedef struct Move {
	
	uint16 val; // Flags - Target Position - Start Position  0000 - 000000 - 000000 
	
	Move(uint16 moveValue) { val = moveValue; }

	Move(uint16 startPos, uint16 targetPos, uint16 flags) {
		assert(startPos <= 63 && targetPos <= 63 && flags <= 15);
		val = startPos | (targetPos << 6) | (flags << 12);
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
} Move;
static_assert(sizeof(Move) == 2);



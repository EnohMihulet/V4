#include <string>

#include "Move.h"
#include "Common.h"
#include "GameState.h"

std::string Move::moveToString() const {
	uint16 start = getStartSquare();
	uint16 target = getTargetSquare();

	std::string moveStr = squareToString(start) + squareToString(target);

	if (isPromotion()) {
		if (isKnightPromotion()) moveStr += 'n';
		else if (isBishopPromotion()) moveStr += 'b';
		else if (isRookPromotion()) moveStr += 'r';
		else if (isQueenPromotion()) moveStr += 'q';
	}

	return moveStr;
}

Move::Move(const GameState& gameState, const std::string& uciMove) {
	assert(uciMove.size() >= 4 && uciMove.size() <= 5);

	auto squareToIndex = [](char file, char rank) -> uint16 {
		assert(file >= 'a' && file <= 'h');
		assert(rank >= '1' && rank <= '8');
		return (rank - '1') * 8 + (file - 'a');
	};

	uint16 startSq = squareToIndex(uciMove[0], uciMove[1]);
	uint8 startFile = startSq % 8;
	uint8 startRank = startSq / 8;
	uint16 targetSq = squareToIndex(uciMove[2], uciMove[3]);
	uint8 targetFile = targetSq % 8;
	uint8 targetRank = targetSq / 8;

	uint16 flags = 0;

	if (uciMove.size() == 5) {
		switch (uciMove[4]) {
			case 'n': flags = KNIGHT_PROMOTE_FLAG; break;
			case 'b': flags = BISHOP_PROMOTE_FLAG; break;
			case 'r': flags = ROOK_PROMOTE_FLAG;   break;
			case 'q': flags = QUEEN_PROMOTE_FLAG;  break;
			default:  flags = 0; break;
		}
	}

	if (gameState.pieceAt(targetSq) != EMPTY) flags |= CAPTURE_FLAG;
	else if ((gameState.pieceAt(startFile) == WPawn || gameState.pieceAt(startSq) == BPawn) && startFile != targetFile && gameState.pieceAt(targetSq) == EMPTY) flags = EN_PASSANT_FLAG;
	else if ((gameState.pieceAt(startSq) == WPawn || gameState.pieceAt(startSq) == BPawn) && abs(startRank - targetRank) == 2) flags = PAWN_TWO_UP_FLAG;

	if (gameState.pieceAt(startSq) == WKing || gameState.pieceAt(startSq) == BKing) {
		if (targetFile - startFile == 2) flags = KING_SIDE_FLAG;
		else if (targetFile - startFile == -2) flags = QUEEN_SIDE_FLAG;
	}

	val = startSq | (targetSq << 6) | (flags << 12);
}

#include <string>

#include "Move.h"

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

Move::Move(const std::string& uciMove) {
	assert(uciMove.size() >= 4 && uciMove.size() <= 5);

	auto squareToIndex = [](char file, char rank) -> uint16 {
		assert(file >= 'a' && file <= 'h');
		assert(rank >= '1' && rank <= '8');
		return (rank - '1') * 8 + (file - 'a');
	};

	uint16 startSq = squareToIndex(uciMove[0], uciMove[1]);
	uint16 targetSq = squareToIndex(uciMove[2], uciMove[3]);
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

	val = startSq | (targetSq << 6) | (flags << 12);
}

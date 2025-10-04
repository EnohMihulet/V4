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

#include <vector>

#include "GameRules.h"

bool isCheck(const GameState& gameState, Color color) {
	if (isSquareAttacked(gameState, (color == White) ? gameState.bitboards[WKing] : gameState.bitboards[BKing], color)) return true;

	return false;
}

GameResult getGameResult(GameState& gameState, std::vector<MoveInfo>& history, Color color) {
	std::vector<Move> moves;
	generateAllMoves(gameState, moves, color, false);
	filterMoves(gameState, history, moves, color);
	if (isCheck(gameState, color) && moves.size() == 0) return color == White ? BlackWin : WhiteWin;
	else if (moves.size() == 0) return Stalemate;
	else if (gameState.halfMoves >= 50) return FiftyMoveRule;
	return InProgress;
}


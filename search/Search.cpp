#include <iostream>
#include <vector>

#include "./Search.h"
#include "../chess/GameState.h"
#include "../chess/GameRules.h"
#include "../movegen/MoveGen.h"
#include "../helpers/GameStateHelper.h"
#include "Evaluation.h"

Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history) {
	
	Move bestMove = NULL_MOVE;
	SearchContext context;
	context.bestMoveThisIteration = NULL_MOVE;
	context.startTime = cntvct();
	context.searchCanceled = false;

	for (int16 depth = 1; depth < 100; depth++) {
		std::cout << depth << std::endl  << std::endl;

		alphaBetaSearch(gameState, history, context, NEG_INF, POS_INF, 0, depth);
		printBoard(gameState);

		if (!context.searchCanceled) {
			bestMove = context.bestMoveThisIteration;
		}

		if (getTimeElapsed(context) >= TIME_PER_MOVE) {
			return bestMove;
		}
	}
	return bestMove;
}

int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining) {
	
	if (pliesRemaining == 0) return evaluate(gameState, gameState.colorToMove);

	if (context.searchCanceled) return 0;

	GameResult result = getGameResult(gameState, history, gameState.colorToMove);
	if (isDraw(result)) return 0;

	// TODO: TRANSPOSITION LOOKUP

	std::vector<Move> moves;
	moves.reserve(64);
	generateAllMoves(gameState, moves, gameState.colorToMove, false);
	filterMoves(gameState, history, moves, gameState.colorToMove);

	// TODO: SORT MOVES

	if (moves.size() == 0) return NEG_INF;

	Move bestMoveInThisPos = 0;
	
	for (const Move& move : moves) {
	
		if (getTimeElapsed(context) >= TIME_PER_MOVE) {
			context.searchCanceled = true;
			return 0;
		}

		gameState.makeMove(move, history);

		int16 eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1);

		gameState.unmakeMove(move, history);

		if (eval > alpha) {
			bestMoveInThisPos = move;
			alpha = eval;

			if (pliesFromRoot == 0) {
				context.bestMoveThisIteration = move;
			}
		}
		if (alpha >= beta) {
			return alpha;
		}
	}
	return alpha;
}

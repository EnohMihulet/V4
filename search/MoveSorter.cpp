#include <iostream>

#include "MoveSorter.h"

#include "../movegen/MoveGen.h"

void printMovesAndScores(GameState& gameState) {
	std::vector<Move> moves;
	std::vector<MoveInfo> history;
	generateAllMoves(gameState, moves, gameState.colorToMove, false);
	filterMoves(gameState, history, moves, gameState.colorToMove);

	PickMoveContext context = {std::vector<uint16>{}, moves[2], moves[3], 0, (uint8)moves.size()};

	scoreMoves(gameState, moves, context);
	for (uint8 i = 0; i < context.size; i++) {
		std::cout << "Move: " << moves[i].moveToString() << " | " 
			<< "Score: " << context.scores[i] << std::endl;
			
	}

	Move move = pickMove(moves, context);
	std::cout << "Best Move: " << move.moveToString() << std::endl;
}

void scoreMoves(GameState& gameState, std::vector<Move>& moves, PickMoveContext& context) {
	context.scores.reserve(context.size);
	for (uint8 i = 0; i < context.size; i++) {
		Move move = moves[i];
		uint16 score = 0;
		score += move.val == context.pvMove.val ? PV_MOVE_SCORE : 0; 
		score += move.val == context.ttMove.val ? TT_MOVE_SCORE : 0;
		if (move.isCapture()) {
			Piece movedPiece = gameState.pieceAt(move.getStartSquare());
			Piece capturedPiece = move.isEnPassant() ? WPawn : gameState.pieceAt(move.getTargetSquare());
			score += BASE_CAPTURE_SCORE + CAPTURE_SCORE_MULT * (STANDARD_PIECE_VALUES[capturedPiece] - STANDARD_PIECE_VALUES[movedPiece]);
		}
		if (move.isPromotion()) {
			switch (move.getFlags()) {
			case QUEEN_PROMOTE_CAPTURE: case QUEEN_PROMOTE_FLAG:
				score += PROMOTION_MULT * STANDARD_PIECE_VALUES[WQueen]; break;
			case ROOK_PROMOTE_CAPTURE: case ROOK_PROMOTE_FLAG:
				score += PROMOTION_MULT * STANDARD_PIECE_VALUES[WRook]; break;
			case KNIGHT_PROMOTE_CAPTURE: case KNIGHT_PROMOTE_FLAG:
				score += PROMOTION_MULT * STANDARD_PIECE_VALUES[WKnight]; break;
			case BISHOP_PROMOTE_CAPTURE: case BISHOP_PROMOTE_FLAG:
				score += PROMOTION_MULT * STANDARD_PIECE_VALUES[WBishop]; break;
			}
		}
		context.scores[i] = score;
	}
}

Move pickMove(std::vector<Move>& moves, PickMoveContext& context) {
	uint8 maxIndex = context.start;
	uint16 maxScore = context.scores[maxIndex];
	for (uint8 i = context.start; i < context.size; i++) {
		if (context.scores[i] > maxScore) {
			maxIndex = i;
			maxScore = context.scores[i];
		}
	}
	std::swap(moves[context.start], moves[maxIndex]);
	std::swap(context.scores[context.start], context.scores[maxIndex]);
	return moves[context.start++];
}


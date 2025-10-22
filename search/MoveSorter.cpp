#include <iostream>

#include "MoveSorter.h"

#include "../movegen/MoveGen.h"

void printMovesAndScores(GameState& gameState) {
	MoveList moves;
	std::vector<MoveInfo> history;
	generateAllMoves(gameState, moves, gameState.colorToMove);

	PickMoveContext context = {ScoreList(), moves.list[2], moves.list[3], 0, (uint8)moves.back};

	scoreMoves(gameState, moves, context);
	for (uint8 i = 0; i < context.size; i++) {
		std::cout << "Move: " << moves.list[i].moveToString() << " | " << "Score: " << context.scores.list[i] << std::endl;
	}

	Move move = pickMove(moves, context);
	std::cout << "Best Move: " << move.moveToString() << std::endl;
}

void scoreMoves(GameState& gameState, MoveList& moves, PickMoveContext& context) {
	for (uint16 i = 0; i < context.size; i++) {
		Move move = moves.list[i];
		uint16 score = 0;
		score += move.val == context.pvMove.val ? PV_MOVE_SCORE : 0; 
		score += move.val == context.ttMove.val ? TT_MOVE_SCORE : 0;
		if (move.isCapture()) { // BUG: moves that are not captures are being marked as captures
			Piece movedPiece = gameState.pieceAt(move.getStartSquare());
			Piece capturedPiece = gameState.pieceAt(move.getTargetSquare());
			if (move.isEnPassant()) capturedPiece = gameState.colorToMove == White ? BPawn : WPawn;
			// TODO: SHOULD NOT NEED TO DO THIS
			if (capturedPiece != EMPTY) score += BASE_CAPTURE_SCORE + CAPTURE_SCORE_MULT * (STANDARD_PIECE_VALUES[capturedPiece] - STANDARD_PIECE_VALUES[movedPiece]);
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
		context.scores.push(score);
	}
}

Move pickMove(MoveList& moves, PickMoveContext& context) {
	uint8 maxIndex = context.start;
	uint16 maxScore = context.scores.list[maxIndex];
	for (uint8 i = context.start; i < context.size; i++) {
		if (context.scores.list[i] > maxScore) {
			maxIndex = i;
			maxScore = context.scores.list[i];
		}
	}
	std::swap(moves.list[context.start], moves.list[maxIndex]);
	std::swap(context.scores.list[context.start], context.scores.list[maxIndex]);
	return moves.list[context.start++];
}


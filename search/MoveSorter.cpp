#include <iostream>

#include "MoveSorter.h"

#include "../movegen/MoveGen.h"
#include "Common.h"

// TODO: Fix this.
void printMovesAndScores(GameState& gameState) {
	MoveList moves;
	std::vector<MoveInfo> history;
	generateAllMoves(gameState, moves, gameState.colorToMove);

	ScoreList scoreList = ScoreList();
	PickMoveContext context = {scoreList, moves.list[2], moves.list[3], MTEntry(), 0, (uint8)moves.back};

	// scoreMoves(gameState, moves, context, );
	for (uint8 i = 0; i < context.size; i++) {
		std::cout << "Move: " << moves.list[i].moveToString() << " | " << "Score: " << context.scores.list[i] << std::endl;
	}

	Move move = pickMove(moves, context);
	std::cout << "Best Move: " << move.moveToString() << std::endl;
}

void scoreMoves(GameState& state, MoveList& moves, PickMoveContext& context, HistoryTable& historyTable, CounterHistoryTable& cHistoryTable, FollowUpHistoryTable& fHistoryTable,
		CounterMoveTable& counterTable, FollowUpMoveTable& followUpTable, ContinuationStack& contStack) {
	ContEntry e;
	ContEntry e2;
	if (contStack.at(0, e) < 0) e = {0,0};
	if (contStack.at(1, e2) < 0) e2 = {0,0}; 
	Move counterMove = counterTable.getMove(contStack);
	Move followUpMove = followUpTable.getMove(contStack); 
	for (uint16 i = 0; i < context.size; i++) {
		Move move = moves.list[i];
		uint8 from = move.getStartSquare();
		uint8 to = move.getTargetSquare();

		if (move.val == context.pvMove.val) {
			context.scores.push(PV_MOVE_SCORE); 
			continue;
      		}
		else if (move.val == context.ttMove.val) {
			context.scores.push(TT_MOVE_SCORE);
			continue;
		}
		else if (move.isPromotion()) {
			uint16 promoRank = (move.isQueenPromotion() ? 3 : move.isRookPromotion() ? 2: move.isBishopPromotion() ? 1 : 0);
			context.scores.push(PROMOTION_BASE + PROMO_STEP * promoRank);
			continue;
		}
		else if (move.isCapture()) {
			Piece movedPiece = state.pieceAt(from);
			Piece capturedPiece = state.pieceAt(to);
			if (move.isEnPassant()) capturedPiece = state.colorToMove == White ? BPawn : WPawn;

			uint16 mvv = STANDARD_PIECE_VALUES[capturedPiece];
			uint16 lva = STANDARD_PIECE_VALUES[movedPiece];
			bool good = (int8)mvv - (int8)lva >= 0;
			uint16 BASE = good ? GOOD_CAPTURE_BASE : BAD_CAPTURE_BASE;

			context.scores.push(BASE + MVV_WEIGHT * (mvv * 16 - lva));
			continue;
		}
		else {
			if (move.val == context.killerMoves.move1.val) {
				context.scores.push(KILLER_MOVE_1_SCORE);
			}
			else if (move.val == counterMove.val) {
				context.scores.push(COUNTER_MOVE_SCORE);
			}
			else if (move.val == followUpMove.val) {
				context.scores.push(FOLLOW_UP_MOVE_SCORE);
			}
			else if (move.val == context.killerMoves.move2.val) {
				context.scores.push(KILLER_MOVE_2_SCORE);
			}
			else {
				Piece p = state.pieceAt(from);
				int16 score = QUIET_BASE + historyTable.getScore(state.colorToMove, from, to);
				score += cHistoryTable.getScore(e, p, to);
				score += fHistoryTable.getScore(e2, p, to);
				context.scores.push(score);
			}
			continue;
		}

		context.scores.push(LOWEST_BASE);
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


#include <cstddef>
#include <iostream>
#include <vector>

#include "Search.h"

#include "Evaluation.h"
#include "MoveSorter.h"
#include "TranspositionTable.h"
#include "../chess/GameState.h"
#include "../chess/GameRules.h"
#include "../helpers/GameStateHelper.h"
#include "../movegen/MoveGen.h"

TranspositionTable g_TranspositionTable;
RepetitionTable g_GameRepetitionHistory;
RepetitionTable g_SearchRepetitionStack;

#ifdef DEBUG_MODE
uint64 g_StartTime = 0;
uint64 g_EndTime = 0;
#endif

void clearTranspositionTable() { g_TranspositionTable.clear_table(); }

#ifdef DEBUG_MODE
#include <iomanip>
#include <sstream>
#include <locale>

#define CLR_RESET	"\033[0m"
#define CLR_TITLE	"\033[36m"
#define CLR_LABEL	"\033[33m"
#define CLR_VALUE	"\033[37m"

struct CommaNum : std::numpunct<char> {
	char do_thousands_sep() const override { return ','; }
	std::string do_grouping() const override { return "\3"; }
};

void printSearchStats(const SearchStats& s, int depth, const Move& bestMove, double elapsed, uint64 zobrist) {
	std::ostringstream ss;
	ss.imbue(std::locale(std::locale(), new CommaNum));

	ss << "\n"
		<< "──────────────────────────────────────────────────────────\n"
		<< CLR_TITLE << "Depth " << depth << CLR_RESET << "\n"
		<< "  Best move: " << CLR_VALUE << bestMove.moveToString() << CLR_RESET << "\n"
		<< "  Time elapsed: " << CLR_VALUE << std::fixed << std::setprecision(2) << elapsed << "ms" << CLR_RESET << "\n"
		<< "  Zobrist: 0x" << std::hex << zobrist << std::dec << "\n"
		<< "──────────────────────────────────────────────────────────\n"
		<< std::setw(28) << std::left << CLR_LABEL "  Nodes searched:" << CLR_RESET << s.nodes << "\n"
		<< std::setw(28) << std::left << CLR_LABEL "  Pruned nodes:" << CLR_RESET << s.prunedNodes << "\n"
		<< std::setw(28) << std::left << CLR_LABEL "  Beta cut-offs:" << CLR_RESET << s.betaCutOffs << "\n"
		<< "──────────────────────────────────────────────────────────\n"
		<< std::setw(28) << std::left << CLR_LABEL "  TT probes:" << CLR_RESET << s.ttProbes << "\n"
		<< std::setw(28) << std::left << CLR_LABEL "  TT hits:" << CLR_RESET << s.ttHits << "\n"
		<< std::setw(28) << std::left << CLR_LABEL "  TT useful hits:" << CLR_RESET << s.ttHitsUseful << "\n"
		<< std::setw(28) << std::left << CLR_LABEL "  TT cutoff hits:" << CLR_RESET << s.ttHitCutoffs << "\n"
		<< "──────────────────────────────────────────────────────────\n"
		<< std::setw(28) << std::left << CLR_LABEL "  TT stores (total):" << CLR_RESET << s.ttStores << "\n"
		<< std::setw(28) << std::left << CLR_LABEL "	• Exact stores:" << CLR_RESET << s.ttStoresExact << "\n"
		<< std::setw(28) << std::left << CLR_LABEL "	• Lower-bound stores:" << CLR_RESET << s.ttStoresLower << "\n"
		<< std::setw(28) << std::left << CLR_LABEL "	• Upper-bound stores:" << CLR_RESET << s.ttStoresUpper << "\n"
		<< "──────────────────────────────────────────────────────────\n";

	std::cout << ss.str() << std::flush;
}


void printSearchTimes(const SearchTimes& t) {
	std::ostringstream ss;
	ss.imbue(std::locale(std::locale(), new CommaNum));

	uint64_t sum_us =
		uint64ToElapsedUS(t.transpositionLookUp) +
		uint64ToElapsedUS(t.transpositionInsertion) +
		uint64ToElapsedUS(t.evaluation) +
		uint64ToElapsedUS(t.gameResultCheck) +
		uint64ToElapsedUS(t.pickContextSetup) +
		uint64ToElapsedUS(t.moveGeneration) +
		uint64ToElapsedUS(t.moveScoring) +
		uint64ToElapsedUS(t.movePicking) +
		uint64ToElapsedUS(t.moveVectorCreation) +
		uint64ToElapsedUS(t.moveMaking) +
		uint64ToElapsedUS(t.moveUnmaking) +
		uint64ToElapsedUS(t.repetitionPush) +
		uint64ToElapsedUS(t.repetitionPop);

	ss << "\n"
		<< "──────────────────────────────────────────────────────────\n"
		<< CLR_TITLE << "Times " << CLR_RESET << "\n"
		<< CLR_LABEL "  Total time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< t.total << "ms" << CLR_RESET << "\n"

		<< CLR_LABEL "  Evaluation time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.evaluation) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Transposition look-up time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.transpositionLookUp) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Transposition insertion time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.transpositionInsertion) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Game state check time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.gameResultCheck) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Pick context setup time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.pickContextSetup) << "μs" << CLR_RESET << "\n"
		<< "──────────────────────────────────────────────────────────\n"

		<< CLR_LABEL "  Move generation time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.moveGeneration) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Move scoring time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.moveScoring) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Move picking time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.movePicking) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Move vector creation time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.moveVectorCreation) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Move making time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.moveMaking) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Move unmaking time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.moveUnmaking) << "μs" << CLR_RESET << "\n"
		<< "──────────────────────────────────────────────────────────\n"

		<< CLR_LABEL "  Repetition push time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.repetitionPush) << "μs" << CLR_RESET << "\n"

		<< CLR_LABEL "  Repetition pop time: " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< uint64ToElapsedUS(t.repetitionPop) << "μs" << CLR_RESET << "\n"
		<< "──────────────────────────────────────────────────────────\n"

		<< CLR_LABEL "  Sum (tracked): " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< static_cast<double>(sum_us) << "μs" << CLR_RESET << "\n"
		<< CLR_LABEL "  Sum (tracked): " << CLR_VALUE << std::fixed << std::setprecision(2)
		<< (static_cast<double>(sum_us) / 1000.0) << "ms" << CLR_RESET << "\n"
		<< "──────────────────────────────────────────────────────────\n";

	std::cout << ss.str() << std::flush;
}

#endif

Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history) {
	Move bestMove;
	SearchContext context;
	context.startTime = cntvct();
	context.searchCanceled = false;
	if (gameState.halfMoves == 0) g_GameRepetitionHistory.clear();

	#ifdef DEBUG_MODE
	SearchStats stats;
	SearchTimes times = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	#endif

	for (int16 depth = 1; depth < 100; depth++) {
		g_SearchRepetitionStack = g_GameRepetitionHistory;

		#ifdef DEBUG_MODE
		alphaBetaSearch(gameState, history, context, NEG_INF, POS_INF, 0, depth, stats, times);

		double elapsed = getTimeElapsed(context.startTime);
		printSearchStats(stats, depth, context.bestMoveThisIteration, elapsed, gameState.zobristHash);
		#else
		alphaBetaSearch(gameState, history, context, NEG_INF, POS_INF, 0, depth);
		#endif

		if (context.searchCanceled) {
			#ifdef DEBUG_MODE
			std::cout << "\nSearch stopped due to time limit.\n";
			uint16 totalTime = getTimeElapsed(context.startTime);
			times.total = totalTime;
			printSearchStats(stats, depth, context.bestMoveThisIteration, totalTime, gameState.zobristHash);
			printSearchTimes(times);
			#endif
			break;
		}
		if (!context.bestMoveThisIteration.isNull()) {
			bestMove = context.bestMoveThisIteration;
		}
	}
	return bestMove;
}

#ifdef DEBUG_MODE
int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, int16 alpha, int16 beta,
					  uint8 pliesFromRoot, uint8 pliesRemaining, SearchStats& stats, SearchTimes& times) {
	stats.nodes++;

	if (pliesRemaining == 0) {
		g_StartTime = cntvct();
		auto eval = evaluate(gameState, gameState.colorToMove);
		times.evaluation += cntvct() - g_StartTime;
		return eval;
	}

	if (context.searchCanceled) return 0;

	stats.ttProbes++;
	g_StartTime = cntvct();
	Entry entry = g_TranspositionTable.table[g_TranspositionTable.index(gameState.zobristHash)];
	if (gameState.zobristHash == entry.zobrist && entry.score != SCORE_SENTINAL) {
		stats.ttHits++;
		if (entry.depth >= pliesRemaining) stats.ttHitsUseful++;

		if (entry.nodeType == Exact && entry.depth >= pliesRemaining) {
			stats.ttHitCutoffs++;
			times.transpositionLookUp += cntvct() - g_StartTime;
			return entry.score;
		} else if (entry.nodeType == LowerBound && entry.score >= beta) {
			stats.ttHitCutoffs++;
			times.transpositionLookUp += cntvct() - g_StartTime;
			return entry.score;
		} else if (entry.nodeType == UpperBound && entry.score <= alpha) {
			stats.ttHitCutoffs++;
			times.transpositionLookUp += cntvct() - g_StartTime;
			return entry.score;
		}

		if (entry.nodeType == LowerBound) alpha = std::max(alpha, entry.score);
		else if (entry.nodeType == UpperBound) beta = std::min(beta, entry.score);
	}
	times.transpositionLookUp += cntvct() - g_StartTime;

	g_StartTime = cntvct();
	std::vector<Move> moves;
	moves.reserve(64);
	times.moveVectorCreation += cntvct() - g_StartTime;
	
	g_StartTime = cntvct();
	generateAllMoves(gameState, moves, gameState.colorToMove);
	times.moveGeneration += cntvct() - g_StartTime;
	uint8 movesSize = moves.size();

	g_StartTime = cntvct();
	if (gameState.halfMoves >= 50) {
		times.gameResultCheck += cntvct() - g_StartTime;
		return 0;
	}
	if (g_SearchRepetitionStack.isRepeated(gameState.zobristHash)) {
		times.gameResultCheck += cntvct() - g_StartTime;
		return 0;
	}
	if (movesSize == 0) {
		// TODO: Insufficient material
		Bitboard kingPos = gameState.colorToMove == White ? gameState.bitboards[WKing] : gameState.bitboards[BKing];
		bool isCheck = isSquareAttacked(gameState, kingPos, gameState.colorToMove == White ? Black : White);
		if (isCheck) {
			if (pliesFromRoot == 0) std::cout << "CHECKMATE" << std::endl;
			times.gameResultCheck += cntvct() - g_StartTime;
			return NEG_INF;
		}


		times.gameResultCheck += cntvct() - g_StartTime;
		return 0;
	}
	times.gameResultCheck += cntvct() - g_StartTime;

	Move bestMoveInThisPos = moves[0];
	int16 originalAlpha = alpha;

	g_StartTime = cntvct();
	PickMoveContext pickMoveContext = {std::vector<uint16>(movesSize),context.bestMoveThisIteration, entry.bestMove, 0, movesSize};
	times.pickContextSetup += cntvct() - g_StartTime;

	g_StartTime = cntvct();
	scoreMoves(gameState, moves, pickMoveContext);
	times.moveScoring += cntvct() - g_StartTime;

	for (uint8 i = 0; i < movesSize; i++) {
		if (getTimeElapsed(context.startTime) >= TIME_PER_MOVE) {
			context.searchCanceled = true;
			return 0;
		}

		g_StartTime = cntvct();
		Move move = pickMove(moves, pickMoveContext);
		times.movePicking += cntvct() - g_StartTime;

		g_StartTime = cntvct();
		gameState.makeMove(move, history);
		times.moveMaking += cntvct() - g_StartTime;

		g_StartTime = cntvct();
		g_SearchRepetitionStack.push(gameState.zobristHash);
		times.repetitionPush += cntvct() - g_StartTime;

		int16 eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1, stats, times);

		g_StartTime = cntvct();
		g_SearchRepetitionStack.pop(gameState.zobristHash);
		times.repetitionPop += cntvct() - g_StartTime;

		g_StartTime = cntvct();
		gameState.unmakeMove(move, history);
		times.moveUnmaking += cntvct() - g_StartTime;

		if (eval > alpha) {
			bestMoveInThisPos = move;
			alpha = eval;

			if (pliesFromRoot == 0) {
				context.bestMoveThisIteration = move;
			}
		}
		if (alpha >= beta) {
			stats.prunedNodes += movesSize - (i+1);
			stats.betaCutOffs += 1;
			break;
		}
	}

	stats.ttStores++;
	g_StartTime = cntvct();
	Entry e{gameState.zobristHash, bestMoveInThisPos, alpha, pliesRemaining, g_TranspositionTable.getNodeType(alpha, beta, originalAlpha)};
	switch (e.nodeType) {
		case Exact: stats.ttStoresExact++; break;
		case LowerBound: stats.ttStoresLower++; break;
		case UpperBound: stats.ttStoresUpper++; break;
	}
	g_TranspositionTable.storeEntry(e);
	times.transpositionInsertion += cntvct() - g_StartTime;
	return alpha;
}
#else
int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, 
					  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining) {

	if (pliesRemaining == 0) return evaluate(gameState, gameState.colorToMove);

	if (context.searchCanceled) return 0;

	Entry entry = g_TranspositionTable.table[g_TranspositionTable.index(gameState.zobristHash)];
	if (gameState.zobristHash == entry.zobrist && entry.score != SCORE_SENTINAL) { 
		if (entry.nodeType == Exact && entry.depth >= pliesRemaining) return entry.score;
		else if (entry.nodeType == LowerBound && entry.score >= beta) return entry.score;
		else if (entry.nodeType == UpperBound && entry.score <= alpha) return entry.score;

	
		if (entry.nodeType == LowerBound) alpha = std::max(alpha, entry.score);
		else if (entry.nodeType == UpperBound) beta = std::min(beta, entry.score);
	}

	std::vector<Move> moves;
	moves.reserve(64);
	generateAllMoves(gameState, moves, gameState.colorToMove);
	uint8 movesSize = moves.size();

	if (gameState.halfMoves >= 50) return 0;

	if (g_SearchRepetitionStack.isRepeated(gameState.zobristHash)) return 0;
	
	if (movesSize == 0) {
		// TODO: Insufficient material
		Bitboard kingPos = gameState.colorToMove == White ? gameState.bitboards[WKing] : gameState.bitboards[BKing];
		bool isCheck = isSquareAttacked(gameState, kingPos, gameState.colorToMove == White ? Black : White);
		if (isCheck) {
			if (pliesFromRoot == 0) std::cout << "CHECKMATE" << std::endl;
			return NEG_INF;
		}

		return 0;
	}

	Move bestMoveInThisPos = moves[0];
	int16 originalAlpha = alpha;

	PickMoveContext pickMoveContext = {std::vector<uint16>(movesSize), context.bestMoveThisIteration, entry.bestMove, 0, movesSize};
	scoreMoves(gameState, moves, pickMoveContext);

	for (uint8 i = 0; i < movesSize; i++) {
		if (getTimeElapsed(context.startTime) >= TIME_PER_MOVE) {
			context.searchCanceled = true;
			return 0;
		}

		Move move = pickMove(moves, pickMoveContext);
		gameState.makeMove(move, history);
		g_SearchRepetitionStack.push(gameState.zobristHash);

		int16 eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1);

		g_SearchRepetitionStack.pop(gameState.zobristHash);
		gameState.unmakeMove(move, history);

		if (eval > alpha) {
			bestMoveInThisPos = move;
			alpha = eval;

			if (pliesFromRoot == 0) context.bestMoveThisIteration = move;
		}
		if (alpha >= beta) break;
	}

	Entry e{gameState.zobristHash, bestMoveInThisPos, alpha, pliesRemaining, g_TranspositionTable.getNodeType(alpha, beta, originalAlpha)};
	g_TranspositionTable.storeEntry(e);
	return alpha;
}
#endif


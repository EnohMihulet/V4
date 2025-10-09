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

TranspositionTable g_TranspositionTable{};

#ifdef DEBUG_MODE
uint64 g_StartTime = 0;
uint64 g_EndTime = 0;
#endif

void clearTranspositionTable() { g_TranspositionTable.clear_table(); }

#ifdef DEBUG_MODE
#include <iomanip>
#include <sstream>
#include <locale>

#define CLR_RESET   "\033[0m"
#define CLR_TITLE   "\033[36m"
#define CLR_LABEL   "\033[33m"
#define CLR_VALUE   "\033[37m"

struct CommaNum : std::numpunct<char> {
	char do_thousands_sep() const override { return ','; }
	std::string do_grouping() const override { return "\3"; }
};

void printSearchStats(const SearchStats& s, int depth, const Move& bestMove, double elapsed, uint64 zobrist) {
	std::ostringstream ss;
	ss.imbue(std::locale(std::locale(), new CommaNum));

	ss << "\n"
	   << "──────────────────────────────────────────────────────────\n"
	   << CLR_TITLE << "  Depth " << depth << CLR_RESET << "\n"
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

	ss << "\n"
	   << "──────────────────────────────────────────────────────────\n"
	   << CLR_TITLE << "Times " << CLR_RESET << "\n"
	   << CLR_LABEL "  Total time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.total << "ms" << CLR_RESET << "\n"
	   << CLR_LABEL "  Evaluation time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.evaluation << "μs" << CLR_RESET << "\n"
	   << CLR_LABEL "  Transposition look-up time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.transpositionLookUp << "μs" << CLR_RESET << "\n"
	   << CLR_LABEL "  Game state check time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.gameStateCheck << "μs" << CLR_RESET << "\n"
	   << "──────────────────────────────────────────────────────────\n"
	   << CLR_LABEL "  Move generation time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.moveGeneration << "μs" << CLR_RESET << "\n"
	   << CLR_LABEL "  Move filtering time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.moveFiltering << "μs" << CLR_RESET << "\n"
	   << CLR_LABEL "  Move scoring time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.moveScoring << "μs" << CLR_RESET << "\n"
	   << CLR_LABEL "  Move picking time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.movePicking << "μs" << CLR_RESET << "\n"
	   << "──────────────────────────────────────────────────────────\n"
	   << CLR_LABEL "  Move making time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.moveMaking << "μs" << CLR_RESET << "\n"
	   << CLR_LABEL "  Move unmaking time: " << CLR_VALUE << std::fixed << std::setprecision(2) << t.moveUnmaking << "μs" << CLR_RESET << "\n"
	   << "──────────────────────────────────────────────────────────\n";

	std::cout << ss.str() << std::flush;
}
#endif

Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history) {

	Move bestMove;
	SearchContext context;
	context.startTime = cntvct();
	context.searchCanceled = false;

	#ifdef DEBUG_MODE
	SearchStats stats;
	SearchTimes times;
	#endif

	for (int16 depth = 1; depth < 100; depth++) {

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
#else
int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, 
			  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining) {
#endif
	if (pliesRemaining == 0) {
		#ifdef DEBUG_MODE
		g_StartTime = cntvct();
		auto eval = evaluate(gameState, gameState.colorToMove);
		times.evaluation += getTimeElapsedUS(g_StartTime);
		return eval;
		#else
		return evaluate(gameState, gameState.colorToMove);
		#endif
	}

	if (context.searchCanceled) return 0;

	#ifdef DEBUG_MODE
	g_StartTime = cntvct();
	GameResult result = getGameResult(gameState, history, gameState.colorToMove);
	times.gameStateCheck += getTimeElapsedUS(g_StartTime);
	#else
	GameResult result = getGameResult(gameState, history, gameState.colorToMove);
	#endif

	if (isDraw(result)) return 0;

	#ifdef DEBUG_MODE 
	stats.ttProbes++;
	g_StartTime = cntvct();
	#endif
	Entry entry = g_TranspositionTable.table[g_TranspositionTable.index(gameState.zobristHash)];
	if (gameState.zobristHash == entry.zobrist && entry.score != SCORE_SENTINAL) { 
		#ifdef DEBUG_MODE
		stats.ttHits++;
		if (entry.depth >= pliesRemaining) stats.ttHitsUseful++;
		#endif

		if (entry.nodeType == Exact && entry.depth >= pliesRemaining) {
			#ifdef DEBUG_MODE
			stats.ttHitCutoffs++;
			#endif
			return entry.score;
		} else if (entry.nodeType == LowerBound && entry.score >= beta) {
			#ifdef DEBUG_MODE
			stats.ttHitCutoffs++;
			#endif
			return entry.score;
		} else if (entry.nodeType == UpperBound && entry.score <= alpha) {
			#ifdef DEBUG_MODE
			stats.ttHitCutoffs++;
			#endif
			return entry.score;
		}
	
		if (entry.nodeType == LowerBound) alpha = std::max(alpha, entry.score);
		else if (entry.nodeType == UpperBound) beta = std::min(beta, entry.score);
	}
	#ifdef DEBUG_MODE
	times.transpositionLookUp += getTimeElapsedUS(g_StartTime);
	#endif

	std::vector<Move> moves;
	moves.reserve(64);
	#ifdef DEBUG_MODE
	g_StartTime = cntvct();
	generateAllMoves(gameState, moves, gameState.colorToMove, false);
	times.moveGeneration += getTimeElapsedUS(g_StartTime);
	g_StartTime = cntvct();
	filterMoves(gameState, history, moves, gameState.colorToMove);
	times.moveFiltering += getTimeElapsedUS(g_StartTime);
	#else
	generateAllMoves(gameState, moves, gameState.colorToMove, false);
	filterMoves(gameState, history, moves, gameState.colorToMove);
	#endif

	if (moves.size() == 0) {
		#ifdef DEBUG_MODE
		if (pliesFromRoot == 0) std::cout << "CHECKMATE" << std::endl;
		#endif
		return NEG_INF;
	}

	Move bestMoveInThisPos = moves[0];
	int16 originalAlpha = alpha;

	uint8 movesSize = moves.size();
	PickMoveContext pickMoveContext = {std::vector<uint16>{},context.bestMoveThisIteration, entry.bestMove, 0, movesSize};
	#ifdef DEBUG_MODE
	g_StartTime = cntvct();
	scoreMoves(gameState, moves, pickMoveContext);
	times.moveScoring += getTimeElapsedUS(g_StartTime);
	#else
	scoreMoves(gameState, moves, pickMoveContext);
	#endif

	for (uint8 i = 0; i < movesSize; i++) {
		if (getTimeElapsed(context.startTime) >= TIME_PER_MOVE) {
			context.searchCanceled = true;
			return 0;
		}

		#ifdef DEBUG_MODE
		g_StartTime = cntvct();
		Move move = pickMove(moves, pickMoveContext);
		times.movePicking += getTimeElapsedUS(g_StartTime);
		g_StartTime = cntvct();
		gameState.makeMove(move, history);
		times.moveMaking += getTimeElapsedUS(g_StartTime);
		#else
		Move move = pickMove(moves, pickMoveContext);
		gameState.makeMove(move, history);
		#endif

		#ifdef DEBUG_MODE
		int16 eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1, stats, times);

		g_StartTime = cntvct();
		gameState.unmakeMove(move, history);
		times.moveUnmaking += getTimeElapsedUS(g_StartTime);
		#else
		int16 eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1);
		gameState.unmakeMove(move, history);
		#endif

		if (eval > alpha) {
			bestMoveInThisPos = move;
			alpha = eval;

			if (pliesFromRoot == 0) {
				context.bestMoveThisIteration = move;
			}
		}
		if (alpha >= beta) {
			#ifdef DEBUG_MODE
			stats.prunedNodes += movesSize - (i+1);
			stats.betaCutOffs += 1;
			#endif
			break;
		}
	}

	Entry e{gameState.zobristHash, bestMoveInThisPos, alpha, pliesRemaining, g_TranspositionTable.getNodeType(alpha, beta, originalAlpha)};
	#ifdef DEBUG_MODE
	stats.ttStores++;
	switch (e.nodeType) {
		case Exact: stats.ttStoresExact++; break;
		case LowerBound: stats.ttStoresLower++; break;
		case UpperBound: stats.ttStoresUpper++; break;
	}
	#endif
	g_TranspositionTable.storeEntry(e);
	return alpha;
}



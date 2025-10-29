#include <cassert>
#include <cstddef>
#include <iostream>
#include <math.h>
#include <vector>

#include "Search.h"

#include "Common.h"
#include "Evaluation.h"
#include "Move.h"
#include "MoveSorter.h"
#include "TranspositionTable.h"
#include "../chess/GameState.h"
#include "../chess/GameRules.h"
#include "../helpers/GameStateHelper.h"
#include "../helpers/Timer.h"
#include "../movegen/MoveGen.h"

TranspositionTable g_TranspositionTable;
MoveTable g_MoveTable;
HistoryTable g_HistoryTable;
CounterMoveTable g_CounterMoveTable;
FollowUpMoveTable g_FollowUpMoveTable;

RepetitionTable g_GameRepetitionHistory;
RepetitionTable g_SearchRepetitionStack;
std::vector<Move> g_MoveStack;

MovePool g_MovePool;
MoveScorePool g_ScoreMovePool;
QuiescencePool g_QuiescencePool;
MoveScorePool g_ScoreQuiesencePool;

uint64 g_StartTime = 0;

#ifdef DEBUG_MODE
#include <iomanip>
#include <sstream>
#include <locale>

#define CLR_RESET	"\033[0m"
#define CLR_TITLE	"\033[36m"
#define CLR_LABEL	"\033[33m"
#define CLR_VALUE	"\033[37m"


void printSearchStats(const SearchStats& s, int depth, const Move& bestMove, double elapsed, uint64 zobrist);

void printSearchTimes(const SearchTimes& t);
#endif

void clearTranspositionTable() { g_TranspositionTable.clearTable(); }

int16 quiescenceSearch(GameState& gameState, std::vector<MoveInfo>& history, Move pvMove, int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining) {
	
	int16 staticEval = evaluate(gameState, gameState.colorToMove);
	if (pliesFromRoot >= 5) return staticEval;

	bool isCheck = isSquareAttacked(gameState, gameState.bitboards[gameState.colorToMove == White ? WKing : BKing], gameState.colorToMove == White ? Black : White);

	int16 bestEval = isCheck ? NEG_INF : staticEval;
	if (!isCheck) {
		if (bestEval >= beta) return bestEval;
		if (bestEval > alpha) alpha = bestEval;
	}

	Entry entry = g_TranspositionTable.table[g_TranspositionTable.index(gameState.zobristHash)];
	if (gameState.zobristHash == entry.zobrist && entry.score != SCORE_SENTINAL && entry.depth == 0) {
		if (entry.nodeType == LowerBound && entry.score >= beta) return entry.score;
		if (entry.nodeType == UpperBound && entry.score <= alpha) return entry.score;
		if (entry.nodeType == LowerBound) alpha = std::max(alpha, entry.score);
		else if (entry.nodeType == UpperBound) beta = std::min(beta, entry.score);
	}

	auto& moves = g_QuiescencePool.getMoveList(pliesFromRoot);
	if (isCheck) generateAllMoves(gameState, moves, gameState.colorToMove);
	else generateAllCaptureMoves(gameState, moves, gameState.colorToMove);

	uint16 movesSize = moves.back;

	if (movesSize == 0) return isCheck ? NEG_INF + pliesFromRoot : 0;

	PickMoveContext pickMoveContext = {g_ScoreMovePool.getScoreList(pliesFromRoot), pvMove, 
					   entry.bestMove, g_MoveTable.table[pliesFromRoot], 0, movesSize};
	scoreMoves(gameState, moves, pickMoveContext, g_HistoryTable, g_CounterMoveTable, g_FollowUpMoveTable, g_MoveStack);
	Move bestMoveInThisPos = moves.list[0];
	int16 originalAlpha = alpha;

	for (uint16 i = 0; i < movesSize; i++) {
		Move move = pickMove(moves, pickMoveContext);
	  		assert(move.val != 0);

		gameState.makeMove(move, history);
		int16 score = -quiescenceSearch(gameState, history, pvMove, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1);
		gameState.unmakeMove(move, history);

		if (score >= beta) {
			Entry e{gameState.zobristHash, move, score, 0, LowerBound};
			g_TranspositionTable.storeEntry(e);
			return score;
		}
		if (score > bestEval) {
			bestMoveInThisPos = move;
			bestEval = score;
		}
		if (score > alpha){
			bestMoveInThisPos = move;
			alpha = score;
		}
	}

	Entry e{gameState.zobristHash, bestMoveInThisPos, alpha, 0, g_TranspositionTable.getNodeType(alpha, beta, originalAlpha)};
	g_TranspositionTable.storeEntry(e);

	return alpha;
}

Move iterativeDeepeningSearch(GameState& gameState, std::vector<MoveInfo>& history) {
	Move bestMove;
	SearchContext context;
	context.startTime = cntvct();
	context.searchCanceled = false;
	if (gameState.halfMoves == 0) g_GameRepetitionHistory.clear();

	#ifdef DEBUG_MODE
	SearchStats stats;
	SearchTimes times;
	#endif

	for (int16 depth = 1; depth < 100; depth++) {
		std::cout << depth << std::endl;
		g_SearchRepetitionStack = g_GameRepetitionHistory;

		#ifdef DEBUG_MODE
		alphaBetaSearch(gameState, history, context, NEG_INF, POS_INF, 0, depth, stats, times);
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
			if (!context.bestMoveThisIteration.isNull() && bestMove.isNull())
				bestMove = context.bestMoveThisIteration;
			break;
		}
		if (!context.bestMoveThisIteration.isNull()) {
			bestMove = context.bestMoveThisIteration;
		}
	}
	return bestMove;
}

// Debug version
int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, int16 alpha, int16 beta,
					  uint8 pliesFromRoot, uint8 pliesRemaining, SearchStats& stats, SearchTimes& times) {
	stats.nodes++;
	stats.plyNodes[pliesFromRoot]++;

	if (pliesRemaining <= 0) {
		g_StartTime = cntvct();
		auto eval = quiescenceSearch(gameState, history, context.bestMoveThisIteration, alpha, beta, 0, 5);
		times.evaluation += cntvct() - g_StartTime;
		return eval;
	}

	if (context.searchCanceled) return 0;

	stats.ttProbes++;
	g_StartTime = cntvct();
	ttLookUpData ttData = g_TranspositionTable.lookUp(gameState.zobristHash, alpha, beta, pliesRemaining, stats);
	times.transpositionLookUp += cntvct() - g_StartTime;

	if (ttData.type == Score) return fromTTScore(ttData.value, pliesFromRoot);
	if (ttData.type == AlphaIncrease) {
		alpha = fromTTScore(ttData.value, pliesFromRoot);
		if (alpha >= beta) return alpha;
	}
	else if (ttData.type == BetaIncrease) {
		beta = fromTTScore(ttData.value, pliesFromRoot);
		if (alpha >= beta) return alpha;
	}

	g_StartTime = cntvct();
	auto& moves = g_MovePool.getMoveList(pliesFromRoot);
	bool isCheck;
	generateAllMoves(gameState, moves, gameState.colorToMove, isCheck);
	times.moveGeneration += cntvct() - g_StartTime;
	uint16 movesSize = moves.back;
	stats.legalMoves[pliesFromRoot] += movesSize;

	g_StartTime = cntvct();
	auto gameResult = getSearchGameResult(gameState, g_SearchRepetitionStack, movesSize, isCheck);
	times.gameResultCheck += cntvct() - g_StartTime;
	if (gameResult == Draw) return 0;
	if (gameResult == Checkmate) return NEG_INF + pliesFromRoot;

	Move bestMoveInThisPos = moves.list[0];
	Move ttMove = g_TranspositionTable.getTTMove(gameState.zobristHash);
	MTEntry killers = g_MoveTable.table[pliesFromRoot];

	int16 originalAlpha = alpha;

	g_StartTime = cntvct();
	PickMoveContext pickMoveContext = {g_ScoreMovePool.getScoreList(pliesFromRoot), context.bestMoveThisIteration, 
					   ttMove, killers, 0, movesSize};
	times.pickContextSetup += cntvct() - g_StartTime;

	int16 historyBonus = pliesRemaining * pliesRemaining;

	g_StartTime = cntvct();
	scoreMoves(gameState, moves, pickMoveContext, g_HistoryTable, g_CounterMoveTable, g_FollowUpMoveTable, g_MoveStack);
	times.moveScoring += cntvct() - g_StartTime;

	bool fullSearched;
	for (uint8 i = 0; i < movesSize; i++) {
		if (getTimeElapsed(context.startTime) >= TIME_PER_MOVE) {
			context.searchCanceled = true;
			return 0;
		}

		g_StartTime = cntvct();
		Move move = pickMove(moves, pickMoveContext);
		times.movePicking += cntvct() - g_StartTime;

		MoveBucket mBucket = getBucketType(gameState, move, context.bestMoveThisIteration, g_TranspositionTable.getTTMove(gameState.zobristHash), pliesFromRoot);

		g_MoveStack.push_back(move);
		g_StartTime = cntvct();
		gameState.makeMove(move, history);
		times.moveMaking += cntvct() - g_StartTime;

		g_StartTime = cntvct();
		g_SearchRepetitionStack.push(gameState.zobristHash);
		times.repetitionPush += cntvct() - g_StartTime;

		int eval;
		uint8 r = getLMR(move, pliesRemaining, i, isCheck, beta != alpha + 1, ttMove, killers, g_HistoryTable.table[gameState.colorToMove][move.getStartSquare()][move.getTargetSquare()]);
		fullSearched = i == 0;
		bool reSearched = false;
		if (i == 0) {
			eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1, stats, times);
		}
		else {
			eval = -alphaBetaSearch(gameState, history, context, -alpha - 1, -alpha, pliesFromRoot + 1, pliesRemaining - 1 - r, stats, times);
			if (eval > alpha) {
				reSearched = true;
				eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1, stats, times);
			}
		}
		fullSearched = fullSearched || reSearched;

		g_StartTime = cntvct();
		g_SearchRepetitionStack.pop(gameState.zobristHash);
		times.repetitionPop += cntvct() - g_StartTime;

		g_StartTime = cntvct();
		gameState.unmakeMove(move, history);
		times.moveUnmaking += cntvct() - g_StartTime;
		g_MoveStack.pop_back();

		stats.bucketTried[mBucket]++;
		
		if (eval > alpha) {
			bestMoveInThisPos = move;
			alpha = eval;

			if (pliesFromRoot == 0) context.bestMoveThisIteration = move;
		}
		if (alpha >= beta) {
			if (!move.isCapture() && fullSearched) {
				g_CounterMoveTable.addMove(gameState, move, g_MoveStack);
				g_FollowUpMoveTable.addMove(gameState, move, g_MoveStack);
				g_MoveTable.storeEntry(pliesFromRoot, move);
				g_HistoryTable.update(gameState.colorToMove, move.getStartSquare(), move.getTargetSquare(), historyBonus);
			}
			stats.prunedNodes += movesSize - (i+1);
			stats.betaCutOffs++;
			stats.cutoffCount[pliesFromRoot]++;
			stats.bucketCutoffs[mBucket]++;
			stats.cutoffIndexSum[pliesFromRoot] += i;
			stats.bucketIndexSum[mBucket] += i;
			if (i == 0) {
				stats.firstMoveCutoffs[pliesFromRoot]++;
				stats.bucketFirstCutoffs[mBucket]++;
			}
			break;
		}
		if (!move.isCapture() && fullSearched) g_HistoryTable.update(gameState.colorToMove, move.getStartSquare(), move.getTargetSquare(), -historyBonus / 16);
	}

	stats.ttStores++;
	g_StartTime = cntvct();
	Entry e{gameState.zobristHash, bestMoveInThisPos, toTTScore(alpha, pliesFromRoot), pliesRemaining, g_TranspositionTable.getNodeType(alpha, beta, originalAlpha)};
	switch (e.nodeType) {
		case Exact: stats.ttStoresExact++; break;
		case LowerBound: stats.ttStoresLower++; break;
		case UpperBound: stats.ttStoresUpper++; break;
	}
	g_TranspositionTable.storeEntry(e);
	times.transpositionInsertion += cntvct() - g_StartTime;
	return alpha;
}

// Release version
int16 alphaBetaSearch(GameState& gameState, std::vector<MoveInfo>& history, SearchContext& context, 
					  int16 alpha, int16 beta, uint8 pliesFromRoot, uint8 pliesRemaining) {

	if (pliesRemaining <= 0) return quiescenceSearch(gameState, history, context.bestMoveThisIteration, alpha, beta, 0, 5);

	if (context.searchCanceled) return 0;

	ttLookUpData ttData = g_TranspositionTable.lookUp(gameState.zobristHash, alpha, beta, pliesRemaining);
	if (ttData.type == Score) return fromTTScore(ttData.value, pliesFromRoot);
	if (ttData.type == AlphaIncrease) {
		alpha = fromTTScore(ttData.value, pliesFromRoot);
		if (alpha >= beta) return alpha;
	}
	else if (ttData.type == BetaIncrease) {
		beta = fromTTScore(ttData.value, pliesFromRoot);
		if (alpha >= beta) return alpha;
	}

	auto& moves = g_MovePool.getMoveList(pliesFromRoot);
	bool isCheck;
	generateAllMoves(gameState, moves, gameState.colorToMove, isCheck);
	uint16 movesSize = moves.back;

	auto gameResult = getSearchGameResult(gameState, g_SearchRepetitionStack, movesSize, isCheck);

	if (gameResult == Draw) return 0;
	if (gameResult == Checkmate) return NEG_INF + pliesFromRoot;

	Move bestMoveInThisPos = moves.list[0];
	Move ttMove = g_TranspositionTable.getTTMove(gameState.zobristHash);
	MTEntry killers = g_MoveTable.table[pliesFromRoot];
	int16 originalAlpha = alpha;
	bool fullSearched;

	PickMoveContext pickMoveContext = {g_ScoreMovePool.getScoreList(pliesFromRoot), context.bestMoveThisIteration, 
					   ttMove, killers, 0, movesSize};

	int16 historyBonus = pliesRemaining*pliesRemaining;
	scoreMoves(gameState, moves, pickMoveContext, g_HistoryTable, g_CounterMoveTable, g_FollowUpMoveTable, g_MoveStack);

	for (uint8 i = 0; i < movesSize; i++) {
		if (getTimeElapsed(context.startTime) >= TIME_PER_MOVE) {
			context.searchCanceled = true;
			return 0;
		}

		Move move = pickMove(moves, pickMoveContext);

		g_MoveStack.push_back(move);
		gameState.makeMove(move, history);
		g_SearchRepetitionStack.push(gameState.zobristHash);

		int eval;
		uint8 r = getLMR(move, pliesRemaining, i, isCheck, beta != alpha + 1, ttMove, killers, g_HistoryTable.table[gameState.colorToMove][move.getStartSquare()][move.getTargetSquare()]);
		fullSearched = (i == 0);
		bool reSearched = false;
		if (i == 0) {
			eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1);
		}
		else {
			eval = -alphaBetaSearch(gameState, history, context, -alpha - 1, -alpha, pliesFromRoot + 1, pliesRemaining - 1 - r);
			if (eval > alpha) {
				reSearched = true;
				eval = -alphaBetaSearch(gameState, history, context, -beta, -alpha, pliesFromRoot + 1, pliesRemaining - 1);
			}
		}
		fullSearched = fullSearched || reSearched;

		g_SearchRepetitionStack.pop(gameState.zobristHash);
		gameState.unmakeMove(move, history);
		g_MoveStack.pop_back();

		if (eval > alpha) {
			bestMoveInThisPos = move;
			alpha = eval;

			if (pliesFromRoot == 0) context.bestMoveThisIteration = move;
		}
		if (alpha >= beta) {
			if (!move.isCapture() && fullSearched) {
				g_CounterMoveTable.addMove(gameState, move, g_MoveStack);
				g_FollowUpMoveTable.addMove(gameState, move, g_MoveStack);
				g_MoveTable.storeEntry(pliesFromRoot, move);
				g_HistoryTable.update(gameState.colorToMove, move.getStartSquare(), move.getTargetSquare(), historyBonus);
			}
			break;
		}
		if (!move.isCapture() && fullSearched) g_HistoryTable.update(gameState.colorToMove, move.getStartSquare(), move.getTargetSquare(), -historyBonus / 16);
	}

	Entry e{gameState.zobristHash, bestMoveInThisPos, toTTScore(alpha, pliesFromRoot), pliesRemaining, g_TranspositionTable.getNodeType(alpha, beta, originalAlpha)};
	g_TranspositionTable.storeEntry(e);
	return alpha;
}

MoveBucket getBucketType(GameState& state, Move move, Move pvMove, Move ttMove, uint8 plies) {
	if (move.val == pvMove.val) return B_PV;
	else if (move.val == ttMove.val) return B_TT;
	else if (move.isPromotion()) return B_Promo;
	else if (move.isCapture()) {
		Piece capturedPiece = state.pieceAt(move.getTargetSquare());
		Piece movedPiece = state.pieceAt(move.getStartSquare());
		uint16 mvv = STANDARD_PIECE_VALUES[capturedPiece];
		uint16 lva = STANDARD_PIECE_VALUES[movedPiece];
		bool good = (int8)mvv - (int8)lva >= 0;
		return good ? B_GoodCap : B_BadCap;
	}
	else {
		MTEntry killerMoves = g_MoveTable.table[plies];
		Move counterMove = g_CounterMoveTable.getMove(state, g_MoveStack);
		Move followUpMove = g_FollowUpMoveTable.getMove(state, g_MoveStack);
		if (move.val == killerMoves.move1.val) return B_Killer1;
		else if (move.val == counterMove.val) return B_Counter;
		else if (move.val == followUpMove.val) return B_FollowUp;
		else if (move.val == killerMoves.move2.val) return B_Killer2;
		return B_QuietHist;
	}
	return B_Other;
}

uint8 getLMR(Move move, uint8 depth, uint8 moveNum, bool isCheck, bool inPV, Move ttMove, MTEntry killers, int16 histScore) {
	if (isCheck) return 0;
	if (move.isCapture() || move.isPromotion()) return 0;
	if (inPV && moveNum <= 2) return 0;
	if (move.val == ttMove.val) return 0;
	if (move.val == killers.move1.val || move.val == killers.move2.val) return 0;
	if (histScore > MAX_HISTORY_BONUS / 2) return 0;

	uint8 d = std::min<uint8>(depth,  MAX_PLY - 1);

	return LMR_TABLE[d][moveNum];
}

#ifdef DEBUG_MODE
struct CommaNum : std::numpunct<char> {
	char do_thousands_sep() const override { return ','; }
	std::string do_grouping() const override { return "\3"; }
};

void printSearchStats(const SearchStats& s, int depth, const Move& bestMove, double elapsed_ms, uint64 zobrist)
{
	using std::left;
	using std::right;
	using std::setw;
	using std::setprecision;
	using std::fixed;

	constexpr int LABEL_W = 30;
	constexpr int VALUE_W = 18;

	auto pct = [](uint64 num, uint64 den) -> double {
		return den ? (100.0 * static_cast<double>(num) / static_cast<double>(den)) : 0.0;
	};

	auto lastNonZeroIndex = [](const uint64* arr, int n) -> int {
		for (int i = n - 1; i >= 0; --i) if (arr[i] != 0) return i;
		return -1;
	};

	auto printList = [&](std::ostringstream& os, const uint64* arr, int n) {
		int last = lastNonZeroIndex(arr, n);
		if (last < 0) { os << "(none)"; return; }
		for (int i = 0; i <= last; ++i) {
			os << arr[i];
			if (i < last) os << ", ";
		}
	};

	static const char* BUCKET_NAMES[B_Count] = {
		"PV move",
		"TT move",
		"Promotions",
		"Good captures",
		"Killer 1",
		"Counter move",
		"Follow-up move",
		"Killer 2",
		"Quiet history",
		"Bad captures",
		"Other"
	};

	std::ostringstream ss;
	ss.imbue(std::locale(std::locale(), new CommaNum));

	ss << "\n"
	   << "──────────────────────────────────────────────────────────\n"
	   << CLR_TITLE << "Depth " << depth << CLR_RESET << "\n"
	   << "  Best move: " << CLR_VALUE << bestMove.moveToString() << CLR_RESET << "\n"
	   << "  Time elapsed: " << CLR_VALUE << fixed << setprecision(2) << elapsed_ms << "ms" << CLR_RESET << "\n";

	{
		auto oldloc = ss.getloc();
		auto oldflags = ss.flags();
		auto oldfill = ss.fill();

		ss.imbue(std::locale::classic());
		ss << "  Zobrist: 0x"
		   << std::hex << std::setw(16) << std::setfill('0') << zobrist
		   << std::dec << std::setfill(oldfill) << "\n";

		ss.imbue(oldloc);
		ss.flags(oldflags);
	}

	const double secs = elapsed_ms > 0.0 ? (elapsed_ms / 1000.0) : 0.0;
	const double nps   = secs > 0.0 ? static_cast<double>(s.nodes) / secs : 0.0;

	ss << "──────────────────────────────────────────────────────────\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  Nodes searched:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.nodes << "\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  Pruned nodes:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.prunedNodes << "\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  Beta cutoffs:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.betaCutOffs << "\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  NPS (nodes/sec):" + CLR_RESET)
	   << setw(VALUE_W) << right << std::fixed << setprecision(0) << nps << "\n";

	const double ttHitRate	  = pct(s.ttHits, s.ttProbes);
	const double ttUsefulRate   = pct(s.ttHitsUseful, s.ttHits);
	const double ttCutoffRate   = pct(s.ttHitCutoffs, s.ttHits);
	const uint64 ttStoreSum	 = s.ttStoresExact + s.ttStoresLower + s.ttStoresUpper;

	ss << "──────────────────────────────────────────────────────────\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  TT probes:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.ttProbes << "\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  TT hits:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.ttHits
	   << "  (" << std::fixed << setprecision(1) << ttHitRate << "%)\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  TT useful hits:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.ttHitsUseful
	   << "  (" << std::fixed << setprecision(1) << ttUsefulRate << "% of hits)\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  TT cutoff hits:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.ttHitCutoffs
	   << "  (" << std::fixed << setprecision(1) << ttCutoffRate << "% of hits)\n"
	   << "──────────────────────────────────────────────────────────\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  TT stores (total):" + CLR_RESET)
	   << setw(VALUE_W) << right << s.ttStores;

	if (ttStoreSum > 0) {
		ss << "  ("
		   << std::fixed << setprecision(1)
		   << pct(s.ttStoresExact, ttStoreSum) << "% exact, "
		   << pct(s.ttStoresLower, ttStoreSum) << "% lower, "
		   << pct(s.ttStoresUpper, ttStoreSum) << "% upper)";
	}
	ss << "\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "	• Exact stores:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.ttStoresExact << "\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "	• Lower-bound stores:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.ttStoresLower << "\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "	• Upper-bound stores:" + CLR_RESET)
	   << setw(VALUE_W) << right << s.ttStoresUpper << "\n";

	const int lastLM   = lastNonZeroIndex(s.legalMoves, MAX_PLY);
	const int lastPN   = lastNonZeroIndex(s.plyNodes, MAX_PLY);
	const int lastCO   = lastNonZeroIndex(s.cutoffCount, MAX_PLY);
	const int lastAny  = std::max({lastLM, lastPN, lastCO});

	std::vector<double> avgCutIdx(std::max(0, lastAny) + 1, 0.0);
	std::vector<double> firstPct(std::max(0, lastAny) + 1, 0.0);
	for (int i = 0; i <= lastAny; ++i) {
		if (i == 0) { avgCutIdx[i] = 0.0; firstPct[i] = 0.0; continue; }
		uint64 cc = s.cutoffCount[i];
		avgCutIdx[i] = (cc > 0) ? static_cast<double>(s.cutoffIndexSum[i]) / static_cast<double>(cc) : 0.0;
		firstPct[i]  = (cc > 0) ? (100.0 * static_cast<double>(s.firstMoveCutoffs[i]) / static_cast<double>(cc)) : 0.0;
	}

	ss << "──────────────────────────────────────────────────────────\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  Per-ply stats:" + CLR_RESET) << "\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "	• Legal moves per ply [" + std::to_string(lastLM + 1) + "]:" + CLR_RESET) << " ";
	printList(ss, s.legalMoves, MAX_PLY); ss << "\n";

	ss << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "	• Nodes per ply [" + std::to_string(lastPN + 1) + "]:" + CLR_RESET) << " ";
	printList(ss, s.plyNodes, MAX_PLY); ss << "\n";

	ss << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "	• Cutoffs per ply [" + std::to_string(lastCO + 1) + "]:" + CLR_RESET) << " ";
	printList(ss, s.cutoffCount, MAX_PLY); ss << "\n";

	if (lastAny >= 0) {
		ss << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "	• Avg cutoff move index:" + CLR_RESET) << " ";
		{
			std::vector<double> tmp = avgCutIdx; 
			ss << std::fixed << setprecision(2);
			for (int i = 0; i <= lastAny; ++i) {
				ss << avgCutIdx[i];
				if (i < lastAny) ss << ", ";
			}
			ss << "\n";
		}
		ss << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "	• First-move cutoffs (%):" + CLR_RESET) << " ";
		{
			ss << std::fixed << setprecision(0);
			for (int i = 0; i <= lastAny; ++i) {
				ss << firstPct[i];
				if (i < lastAny) ss << ", ";
			}
			ss << "\n";
		}
	}

	ss << "──────────────────────────────────────────────────────────\n"
	   << setw(LABEL_W) << left << (std::string(CLR_LABEL) + "  Per-bucket stats:" + CLR_RESET) << "\n";

	ss << "	" << setw(18) << left << "Bucket"
	   << setw(18) << right << "Tried"
	   << setw(18) << right << "Cutoffs"
	   << setw(24) << right << "Avg cutoff idx"
	   << setw(24) << right << "First-move (%)"
	   << "\n";

	for (int i = 0; i < B_Count; ++i) {
		const uint64 tried   = s.bucketTried[i];
		const uint64 cuts	= s.bucketCutoffs[i];
		const double avgIdx  = cuts ? static_cast<double>(s.bucketIndexSum[i]) / static_cast<double>(cuts) : 0.0;
		const double firstP  = cuts ? (100.0 * static_cast<double>(s.bucketFirstCutoffs[i]) / static_cast<double>(cuts)) : 0.0;

		ss << "	" << setw(18) << left  << BUCKET_NAMES[i]
		   << setw(18) << right << tried
		   << setw(18) << right << cuts
		   << setw(20) << right << (std::ostringstream() << std::fixed << setprecision(2) << avgIdx, ss.str().erase(ss.str().size()-ss.str().size()))
		   << std::fixed << setprecision(2);
		{
			std::ostringstream tmp;
			tmp.imbue(ss.getloc());
			tmp << std::fixed << setprecision(2) << avgIdx;
			ss << tmp.str();
		}
		{
			std::ostringstream tmp;
			tmp.imbue(ss.getloc());
			tmp << std::fixed << setprecision(1) << firstP;
			ss << setw(24) << right << tmp.str() << "\n";
		}
	}

	ss << "──────────────────────────────────────────────────────────\n";

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



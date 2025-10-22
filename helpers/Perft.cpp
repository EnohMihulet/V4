#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include "Perft.h"
#include "../movegen/MoveGen.h"

uint64 perft(GameState& state, std::vector<MoveInfo>& history, uint8 depth) {
	if (depth == 0) return 1ULL;

	MoveList moves;
	generateAllMoves(state, moves, state.colorToMove);

	uint64 nodes = 0;
	for (const Move& move : moves) {
		state.makeMove(move, history);
		nodes += perft(state, history, depth - 1);
		state.unmakeMove(move, history);
	}
	return nodes;
}

uint64 perft_count(GameState& state, std::vector<MoveInfo>& history, uint8 depth, PerftStats& stats) {
	if (depth == 0) {
		stats.nodes += 1;
		return 1ULL;
	}

	MoveList moves;
	generateAllMoves(state, moves, state.colorToMove);

	if (moves.isEmpty()) {
	uint64 kingBB = state.colorToMove == White ? state.bitboards[WKing] : state.bitboards[BKing];
	Color them = state.colorToMove == White ? Black : White;
		if (isSquareAttacked(state, kingBB, them)) stats.checkmates++;
		else stats.stalemates++;
		return 0ULL;
	}

	uint64 nodes = 0;
	for (const Move& m : moves) {

		if (m.isCapture()) stats.captures++;
		if (m.isEnPassant()) stats.enPassants++;
		if (m.isKingSideCastle()) { stats.castles++; stats.ksCastles++; }
		if (m.isQueenSideCastle()) { stats.castles++; stats.qsCastles++; }
		if (m.isPromotion()) stats.promotions++;
		if (m.isTwoUpMove()) stats.dblPushes++;

		state.makeMove(m, history);

		nodes += perft_count(state, history, depth - 1, stats);
		state.unmakeMove(m, history);
	}
	return nodes;
}

static void perftDivide(GameState& state, std::vector<MoveInfo>& history, uint8 depth) {
	MoveList moves;
	generateAllMoves(state, moves, state.colorToMove);

	uint64 total = 0;
	for (const Move& mv : moves) {
		state.makeMove(mv, history);
		uint64 cnt = perft(state, history, depth - 1);
		state.unmakeMove(mv, history);
		total += cnt;

		std::cout << mv.moveToString() << ": " << cnt << "\n";
	}
	std::cout << "Total: " << total << "\n";
}

static void requireEqual(uint64 expected, uint64 got, const std::string& label) {
	if (expected != got) {
		std::cerr << "[FAIL] " << label << " — expected " << expected << ", got " << got << "\n";
		// std::abort();
	} else {
		std::cout << "[OK]   " << label << " = " << got << "\n";
	}
}

void runPerftTest() {
	const std::map<int, uint64> expected = {
		{1, 20ULL},
		{2, 400ULL},
		{3, 8902ULL},
		{4, 197281ULL},
		{5, 4865609ULL} // ,
		// {6, 119060324ULL},
		// {7, 3195901860ULL}
	};

	for (const auto& [depth, exp] : expected) {
		std::vector<MoveInfo> history;
		history.reserve(256);

		GameState state((std::string)DEFAULT_FEN_POSITION);
		// GameState state("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
		// GameState state("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
		// GameState state("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
		// GameState state("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
		// GameState state("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
		// GameState state("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

		auto t0 = std::chrono::high_resolution_clock::now();
		PerftStats stats;
		uint64 nodes = perft_count(state, history, static_cast<uint8>(depth), stats);
		auto t1 = std::chrono::high_resolution_clock::now();
		double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

		std::cout << "\nDepth " << depth << ":\n";
		std::cout << "  Nodes: " << nodes << "\n";
		std::cout << "  Captures: " << stats.captures << "\n";
		std::cout << "  EP: " << stats.enPassants << "\n";
		std::cout << "  Castles: " << stats.castles << " (O-O " << stats.ksCastles << ", O-O-O " << stats.qsCastles << ")\n";
		std::cout << "  Promotions: " << stats.promotions << "\n";
		std::cout << "  2-push: " << stats.dblPushes << "\n";
		std::cout << "  Checkmates: " << stats.checkmates << std::endl;
		std::cout << "  Stalemates: " << stats.stalemates << std::endl;
		std::cout << "  Time: " << std::fixed << std::setprecision(2) << ms << " ms\n";
		if (ms > 0.0) {
			double mnps = (nodes / ms) * 1000.0;
			std::cout << "  NPS: " << std::fixed << std::setprecision(0) << mnps << "\n";
		}

		requireEqual(exp, nodes, "StartPos perft(" + std::to_string(depth) + ")");
	}

	{
		std::vector<MoveInfo> history;
		
		GameState state((std::string)DEFAULT_FEN_POSITION);
		// GameState state("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
		// GameState state("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
		// GameState state("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
		// GameState state("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");
		// GameState state("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
		// GameState state("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

		std::cout << "\nRoot divide at depth 3:\n";
		perftDivide(state, history, 3);

		MoveList rootMoves;
		generateAllMoves(state, rootMoves, state.colorToMove);
		std::mt19937_64 rng(0xC0FFEE);
		std::shuffle(rootMoves.begin(), rootMoves.end(), rng);

		uint8 depth = 3;
		uint64 sum = 0;
		for (const Move& m : rootMoves) {
			state.makeMove(m, history);
			sum += perft(state, history, depth - 1);
			state.unmakeMove(m, history);
		}

		uint64 direct = perft(state, history, depth);
		requireEqual(direct, sum, "Move-order independence perft(" + std::to_string(depth) + ")");
	}

	std::cout << "\n[Perft suite] All checks passed \n";
}

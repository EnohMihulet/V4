#include <iostream>
#include <string>

#include "EvaluationTests.h"
#include "Evaluation.h"
#include "../chess/GameState.h"
#include "../helpers/timer.h"
#include "../helpers/GameStateHelper.h"

void testIsolatePawn() {
	std::string fen = "8/8/8/8/8/8/8/P1P5 w - - 0 1"; 
	GameState state(fen);
	printBitboard(state.bitboards[WPawn]);

	int16 eval = 0;
	{
		ScopedTimer timer("Isolated Pawn Test");
		eval = evaluate(state, White);
	}

	std::cout << "FEN: " << fen << "\n";
	std::cout << "Eval: " << eval << "\n";

	int16 expected = PIECE_VALUES[WPawn]*2 - ISOLATED_PAWNS;
	int16 delta = 50;
	int16 expectedMin = expected - delta;
	int16 expectedMax = expected + delta;

	if (eval >= expectedMin && eval <= expectedMax) {
		std::cout << "Passed\n";
	} else {
		std::cout << "Failed (expected " << expectedMin << "–" << expectedMax << ")\n";
	}
}

void testPassedPawn() {
	std::string fen = "8/8/8/8/8/8/3P4/3p4 w - - 0 1"; 
	GameState state(fen);
	printBitboard(state.bitboards[WPawn]);
	printBitboard(state.bitboards[BPawn]);

	int16 eval = 0;
	{
		ScopedTimer timer("Passed Pawn Test");
		eval = evaluate(state, White);
	}

	std::cout << "FEN: " << fen << "\n";
	std::cout << "Eval: " << eval << "\n";

	int16 expected = 250;
	int16 delta = 150;
	int16 expectedMin = expected - delta;
	int16 expectedMax = expected + delta;

	if (eval >= expectedMin && eval <= expectedMax) {
		std::cout << "Passed\n";
	} else {
		std::cout << "Failed (expected " << expectedMin << "–" << expectedMax << ")\n";
	}
}

void testKnightPair() {
	std::string fen = "8/8/8/8/8/8/8/NN6 w - - 0 1"; 
	GameState state(fen);
	printBitboard(state.bitboards[WKnight]);

	int16 eval = 0;
	{
		ScopedTimer timer("Knight Pair Test");
		eval = evaluate(state, White);
	}

	std::cout << "FEN: " << fen << "\n";
	std::cout << "Eval: " << eval << "\n";

	int16 expected = PIECE_VALUES[WKnight]*2 + KNIGHT_PAIR;
	int16 delta = 100;
	int16 expectedMin = expected - delta;
	int16 expectedMax = expected + delta;

	if (eval >= expectedMin && eval <= expectedMax) {
		std::cout << "Passed\n";
	} else {
		std::cout << "Failed (expected " << expectedMin << "–" << expectedMax << ")\n";
	}
}

void testKingSafety() {
	std::string fen = "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"; 
	GameState state(fen);
	printBitboard(state.bitboards[WKing]);
	printBitboard(state.bitboards[BKing]);

	int16 eval = 0;
	{
		ScopedTimer timer("King Safety Test");
		eval = evaluate(state, White);
	}

	std::cout << "FEN: " << fen << "\n";
	std::cout << "Eval: " << eval << "\n";

	int16 expected = 0;
	int16 delta = 50;
	int16 expectedMin = expected - delta;
	int16 expectedMax = expected + delta;

	if (eval >= expectedMin && eval <= expectedMax) {
		std::cout << "Passed\n";
	} else {
		std::cout << "Failed (expected " << expectedMin << "–" << expectedMax << ")\n";
	}
}

void testDoubledPawns() {
	std::string fen = "8/1p6/8/8/8/8/1P6/1P6 w - - 0 1"; 
	GameState state(fen);
	printBitboard(state.bitboards[WPawn]);

	int16 eval = 0;
	{
		ScopedTimer timer("Doubled Pawn Test");
		eval = evaluate(state, White);
	}

	std::cout << "FEN: " << fen << "\n";
	std::cout << "Eval: " << eval << "\n";

	int16 expected = PIECE_VALUES[WPawn] + DOUBLED_PAWNS*2;
	int16 delta = 50;
	int16 expectedMin = expected - delta;
	int16 expectedMax = expected + delta;

	if (eval >= expectedMin && eval <= expectedMax) {
		std::cout << "Passed\n";
	} else {
		std::cout << "Failed (expected " << expectedMin << "–" << expectedMax << ")\n";
	}
}

void testStartingPosition() {
	std::string fen = (std::string) DEFAULT_FEN_POSITION;
	GameState state(fen);

	int16 wEval = 0;
	int16 bEval = 0;
	wEval = evaluate(state, White);
	bEval = evaluate(state, Black);

	std::cout << "FEN: " << fen << "\n";
	std::cout << "White Eval: " << wEval << "\n";
	std::cout << "Black Eval: " << bEval << "\n";

	int16 expected = 0;
	int16 delta = 50;
	int16 expectedMin = expected - delta;
	int16 expectedMax = expected + delta;

	if (wEval - bEval >= expectedMin && wEval - bEval <= expectedMax) {
		std::cout << "Passed\n";
	} else {
		std::cout << "Failed (expected " << expectedMin << "–" << expectedMax << ")\n";
	}
}

void testEqualPositions() {
	for (const std::string_view fen : FIVE_MIRRORED_FENS) {

		GameState state((std::string) fen);

		int16 wEval = 0;
		int16 bEval = 0;
		wEval = evaluate(state, White);
		bEval = evaluate(state, Black);

		std::cout << "FEN: " << fen << "\n";
		std::cout << "White Eval: " << wEval << "\n";
		std::cout << "Black Eval: " << bEval << "\n";

		int16 expected = 0;
		int16 delta = 50;
		int16 expectedMin = expected - delta;
		int16 expectedMax = expected + delta;

		if (wEval - bEval >= expectedMin && wEval - bEval <= expectedMax) {
			std::cout << "Passed\n";
		} else {
			std::cout << "Failed (expected " << expectedMin << "–" << expectedMax << ")\n";
		}
	}
}

#include <iostream>

#include "GameStateHelper.h"
#include "../movegen/MoveGen.h"


void printBitboard(Bitboard bb) {
	std::cout << "Bitboard: " << bb << std::endl;
	for (int rank = 7; rank >= 0; --rank) {
		for (int file = 0; file < 8; ++file) {
			int square = rank * 8 + file;
			std::cout << ((bb >> square) & 1ULL ? "1 " : ". ");
		}
		std::cout << " " << (rank + 1) << std::endl;
	}
	std::cout << "a b c d e f g h\n" << std::endl;
}

void printGameStateBitboards(const GameState& state) {
	for (size_t i = 0; i < state.bitboards.size(); ++i) {
		std::cout << "Bitboard[" << i << "]:" << std::endl;
		printBitboard(state.bitboards[i]);
	}
}

void printBoard(const GameState& state) {
	std::cout << "\nCurrent Board:\n\n";

	for (int rank = 7; rank >= 0; --rank) {
		for (int file = 0; file < 8; ++file) {
			int sq = rank * 8 + file;
			Piece piece = state.board[sq];
			char symbol = '.';

			switch (piece) {
				case WPawn:   symbol = 'P'; break;
				case WKnight: symbol = 'N'; break;
				case WBishop: symbol = 'B'; break;
				case WRook:   symbol = 'R'; break;
				case WQueen:  symbol = 'Q'; break;
				case WKing:   symbol = 'K'; break;
				case BPawn:   symbol = 'p'; break;
		case BKnight: symbol = 'n'; break;
				case BBishop: symbol = 'b'; break;
				case BRook:   symbol = 'r'; break;
				case BQueen:  symbol = 'q'; break;
				case BKing:   symbol = 'k'; break;
				default: symbol = '.'; break;
			}

			std::cout << symbol << ' ';
		}
		std::cout << " " << (rank + 1) << '\n';
	}

	std::cout << "a b c d e f g h\n" << std::endl;
}

bool gameStatesAreEqual(const GameState& a, const GameState& b) {
	if (a.board != b.board) return false;
	if (a.bitboards != b.bitboards) return false;
	if (a.zobristHash != b.zobristHash) return false;
	if (a.castlingRights != b.castlingRights) return false;
	if (a.enPassantFile != b.enPassantFile) return false;
	if (a.halfMoves != b.halfMoves) return false;
	if (a.fullMoves != b.fullMoves) return false;
	if (a.colorToMove != b.colorToMove) return false;
	return true;
}

void testMakeUnmake(GameState& startState, const Move& move, const std::string& description) {
	GameState state = startState;
	std::vector<MoveInfo> history;

	state.makeMove(move, history);
	state.unmakeMove(move, history);

	if (gameStatesAreEqual(startState, state)) {
		PASS(description);
	} else {
		FAIL(description);
		std::cout << "\n--- FAILURE DETAILS: " << description << " ---\n";
		std::cout << "\nBefore:\n";
		printBoard(startState);
		std::cout << "\nAfter:\n";
		printBoard(state);
		std::cout << "--------------------------------------\n";
	}
}

void testMakeUnmakePawns() {
	std::cout << "\n=== Pawn Tests ===\n";
	{
		GameState state("8/8/8/8/8/8/4P3/8 w - - 0 1");
		Move move(E2, E3, NO_FLAG);
		testMakeUnmake(state, move, "White pawn single push E2->E3");
	}
	{
		GameState state("8/8/8/8/8/8/4P3/8 w - - 0 1");
		Move move(E2, E4, PAWN_TWO_UP_FLAG);
		testMakeUnmake(state, move, "White pawn double push E2->E4");
	}
	{
		GameState state("8/8/8/3p4/4P3/8/8/8 w - - 0 1");
		Move move(E4, D5, CAPTURE_FLAG);
		testMakeUnmake(state, move, "White pawn capture left E4->D5");
	}
	{
		GameState state("8/8/8/5p2/4P3/8/8/8 w - - 0 1");
		Move move(E4, F5, CAPTURE_FLAG);
		testMakeUnmake(state, move, "White pawn capture right E4->F5");
	}
	{
		GameState state("8/4P3/8/8/8/8/8/8 w - - 0 1");
		Move move(E7, E8, KNIGHT_PROMOTE_FLAG);
		testMakeUnmake(state, move, "White pawn promote to knight E7->E8");
	}
	{
		GameState state("8/4P3/8/8/8/8/8/8 w - - 0 1");
		Move move(E7, E8, QUEEN_PROMOTE_FLAG);
		testMakeUnmake(state, move, "White pawn promote to queen E7->E8");
	}
	{
		GameState state("8/4p3/8/8/8/8/8/8 b - - 0 1");
		Move move(E7, E6, NO_FLAG);
		testMakeUnmake(state, move, "Black pawn single push E7->E6");
	}
	{
		GameState state("8/4p3/8/8/8/8/8/8 b - - 0 1");
		Move move(E7, E5, PAWN_TWO_UP_FLAG);
		testMakeUnmake(state, move, "Black pawn double push E7->E5");
	}
	{
		GameState state("8/8/8/4p3/3P4/8/8/8 b - - 0 1");
		Move move(E5, D4, CAPTURE_FLAG);
		testMakeUnmake(state, move, "Black pawn capture left E5->D4");
	}
	{
		GameState state("8/8/8/4p3/5P2/8/8/8 b - - 0 1");
		Move move(E5, F4, CAPTURE_FLAG);
		testMakeUnmake(state, move, "Black pawn capture right E5->F4");
	}
	{
		GameState state("8/8/8/8/8/8/4p3/8 b - - 0 1");
		Move move(E2, E1, KNIGHT_PROMOTE_FLAG);
		testMakeUnmake(state, move, "Black pawn promote to knight E2->E1");
	}
	{
		GameState state("8/8/8/8/8/8/4p3/8 b - - 0 1");
		Move move(E2, E1, QUEEN_PROMOTE_FLAG);
		testMakeUnmake(state, move, "Black pawn promote to queen E2->E1");
	}
}

void testMakeUnmakeKnights() {
	std::cout << "\n=== Knight Tests ===\n";
	{
		GameState state("8/8/8/8/8/8/6N1/8 w - - 0 1");
		Move move(G2, F4, NO_FLAG);
		testMakeUnmake(state, move, "Knight move G2->F4");
	}
	{
		GameState state("8/8/8/8/8/8/1N6/8 w - - 0 1");
		Move move(B2, C4, NO_FLAG);
		testMakeUnmake(state, move, "Knight move B2->C4");
	}
	{
		GameState state("8/8/8/8/8/5N2/8/8 w - - 0 1");
		Move move(F3, D4, NO_FLAG);
		testMakeUnmake(state, move, "Knight move F3->D4");
	}
	{
		GameState state("8/8/8/6N1/8/8/8/8 w - - 0 1");
		Move move(G5, E4, NO_FLAG);
		testMakeUnmake(state, move, "Knight move G5->E4");
	}
	{
		GameState state("8/8/8/8/8/2N5/8/8 w - - 0 1");
		Move move(C3, E4, NO_FLAG);
		testMakeUnmake(state, move, "Knight move C3->E4");
	}
}

void testMakeUnmakeBishops() {
	std::cout << "\n=== Bishop Tests ===\n";
	{
		GameState state("8/8/8/8/8/8/8/2B5 w - - 0 1");
		Move move(C1, H6, NO_FLAG);
		testMakeUnmake(state, move, "Bishop long diagonal C1->H6");
	}
	{
		GameState state("8/8/8/3B4/8/8/8/8 w - - 0 1");
		Move move(D5, F7, NO_FLAG);
		testMakeUnmake(state, move, "Bishop diagonal D5->F7");
	}
	{
		GameState state("8/8/8/8/8/8/5B2/8 w - - 0 1");
		Move move(F2, C5, NO_FLAG);
		testMakeUnmake(state, move, "Bishop diagonal F2->C5");
	}
	{
		GameState state("8/8/8/8/8/8/2B5/8 w - - 0 1");
		Move move(C2, H7, NO_FLAG);
		testMakeUnmake(state, move, "Bishop diagonal C2->H7");
	}
}

void testMakeUnmakeRooks() {
	std::cout << "\n=== Rook Tests ===\n";
	{
		GameState state("8/8/8/8/8/8/8/R7 w - - 0 1");
		Move move(A1, A4, NO_FLAG);
		testMakeUnmake(state, move, "Rook vertical A1->A4");
	}
	{
		GameState state("8/8/8/8/8/8/8/7R w - - 0 1");
		Move move(H1, H4, NO_FLAG);
		testMakeUnmake(state, move, "Rook vertical H1->H4");
	}
	{
		GameState state("8/8/8/8/8/8/8/R7 w - - 0 1");
		Move move(A1, D1, NO_FLAG);
		testMakeUnmake(state, move, "Rook horizontal A1->D1");
	}
	{
		GameState state("8/8/8/3R4/8/8/8/8 w - - 0 1");
		Move move(D5, D8, NO_FLAG);
		testMakeUnmake(state, move, "Rook vertical D5->D8");
	}
}

void testMakeUnmakeQueens() {
	std::cout << "\n=== Queen Tests ===\n";
	{
		GameState state("8/8/8/8/8/8/8/3Q4 w - - 0 1");
		Move move(D1, H5, NO_FLAG);
		testMakeUnmake(state, move, "Queen diagonal D1->H5");
	}
	{
		GameState state("8/8/8/8/8/8/8/3Q4 w - - 0 1");
		Move move(D1, D4, NO_FLAG);
		testMakeUnmake(state, move, "Queen vertical D1->D4");
	}
	{
		GameState state("8/8/8/8/8/8/8/3Q4 w - - 0 1");
		Move move(D1, A4, NO_FLAG);
		testMakeUnmake(state, move, "Queen diagonal D1->A4");
	}
	{
		GameState state("8/8/8/3Q4/8/8/8/8 w - - 0 1");
		Move move(D5, H5, NO_FLAG);
		testMakeUnmake(state, move, "Queen horizontal D5->H5");
	}
}

void testMakeUnmakeKings() {
	std::cout << "\n=== King Tests ===\n";
	{
		GameState state("8/8/8/8/8/8/8/4K3 w KQ - 0 1");
		Move move(E1, F1, NO_FLAG);
		testMakeUnmake(state, move, "King move E1->F1");
	}
	{
		GameState state("8/8/8/8/8/8/8/4K3 w KQ - 0 1");
		Move move(E1, D1, NO_FLAG);
		testMakeUnmake(state, move, "King move E1->D1");
	}
	{
		GameState state("8/8/8/8/8/8/8/4K2R w K - 0 1");
		Move move(E1, G1, KING_SIDE_FLAG);
		testMakeUnmake(state, move, "King side castle E1->G1");
	}
	{
		GameState state("8/8/8/8/8/8/8/R3K3 w Q - 0 1");
		Move move(E1, C1, QUEEN_SIDE_FLAG);
		testMakeUnmake(state, move, "Queen side castle E1->C1");
	}
	{
		GameState state("r3k2r/8/8/8/8/8/8/8 b kq - 0 1");
		Move move(E8, G8, KING_SIDE_FLAG);
		testMakeUnmake(state, move, "Black king side castle E8->G8");
	}
	{
		GameState state("r3k2r/8/8/8/8/8/8/8 b kq - 0 1");
		Move move(E8, C8, QUEEN_SIDE_FLAG);
		testMakeUnmake(state, move, "Black queen side castle E8->C8");
	}
}

void testAllMoves(GameState state) {
	std::cout << "\n=== Full Move Regression ===\n";
	std::vector<MoveInfo> history;
	std::vector<Move> moves;
	moves.reserve(64);
	generateAllMoves(state, moves, state.colorToMove);
	filterMoves(state, history, moves, state.colorToMove);

	int passed = 0, failed = 0;

	for (const Move& move : moves) {
		GameState copy = state;
		copy.makeMove(move, history);
		copy.unmakeMove(move, history);

		if (gameStatesAreEqual(state, copy)) {
			passed++;
		} else {
			failed++;
			std::cout << "Mismatch for move: " << move.moveToString() << std::endl;
		}
	}

	std::cout << "Moves tested: " << moves.size()
		  << " | Passed: " << passed
		  << " | Failed: " << failed << std::endl;
}

void testMakeUnmakeMove() {
	testMakeUnmakePawns();
	testMakeUnmakeKnights();
	testMakeUnmakeBishops();
	testMakeUnmakeRooks();
	testMakeUnmakeQueens();
	testMakeUnmakeKings();

	// testAllMoves(state);
}

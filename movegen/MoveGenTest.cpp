#include <chrono>
#include <iostream>
#include <vector>

#include "MoveGen.h"
#include "MoveGenTest.h"
#include "../helpers/Timer.h"

void generatePieceMoves(GameState& gameState, std::vector<Move>& moves, Piece piece) {

	switch (piece) {
	case WPawn: generatePawnMoves(gameState, moves, White, false); break;
	case BPawn: generatePawnMoves(gameState, moves, Black, false); break;
	case WKnight: generateKnightMoves(gameState, moves, White, false); break;
	case BKnight: generateKnightMoves(gameState, moves, Black, false); break;
	case WBishop: generateBishopMoves(gameState, moves, White, false); break;
	case BBishop: generateBishopMoves(gameState, moves, Black, false); break;
	case WRook: generateRookMoves(gameState, moves, White, false); break;
	case BRook: generateRookMoves(gameState, moves, Black, false); break;
	case WQueen: generateQueenMoves(gameState, moves, White, false); break;
	case BQueen: generateQueenMoves(gameState, moves, Black, false); break;
	case WKing: generateKingMoves(gameState, moves, White, false); break;
	case BKing: generateKingMoves(gameState, moves, Black, false); break;
	default:
		std::cerr << "Invalid piece type passed to PrintPieceMoves\n";
		return;
	}
}


void printPieceMoves(GameState& gameState, Piece piece) {
	std::vector<Move> moves;
	std::vector<MoveInfo> history;
	Color color = isWhite(piece) ? White : Black;

	generatePieceMoves(gameState, moves, piece);

	std::cout << "Psuedo Legal Moves for piece " << (int)piece << ":\n";
	for (const Move& move : moves) {
		int from = move.getStartSquare();
		int to   = move.getTargetSquare();

		char fromFile = 'a' + (from % 8);
		char fromRank = '1' + (from / 8);
		char toFile   = 'a' + (to % 8);
		char toRank   = '1' + (to / 8);

		std::cout << fromFile << fromRank << " -> " 
				  << toFile << toRank 
				  << " (flags=" << move.getFlags() << ")\n";
	}

	filterMoves(gameState, history, moves, color);

	std::cout << "Legal Moves for piece " << (int)piece << ":\n";
	for (const Move& move : moves) {
		int from = move.getStartSquare();
		int to   = move.getTargetSquare();

		char fromFile = 'a' + (from % 8);
		char fromRank = '1' + (from / 8);
		char toFile   = 'a' + (to % 8);
		char toRank   = '1' + (to / 8);

		std::cout << fromFile << fromRank << " -> " 
				  << toFile << toRank 
				  << " (flags=" << move.getFlags() << ")\n";
	}
}


void printPsuedoLegalMoves(GameState& gameState, Color color) {
	std::vector<Move> moves;
	generateAllMoves(gameState, moves, color, false);

	std::string colorStr = (color == White) ? "White" : "Black";
	std::cout << "Psuedo Legal Moves for " << colorStr << ":\n";
	for (const Move& move : moves) {
		int from = move.getStartSquare();
		int to   = move.getTargetSquare();

		char fromFile = 'a' + (from % 8);
		char fromRank = '1' + (from / 8);
		char toFile   = 'a' + (to % 8);
		char toRank   = '1' + (to / 8);

		std::cout << fromFile << fromRank << " -> " 
				  << toFile << toRank 
				  << " (flags=" << move.getFlags() << ")\n";
	}
}

void printLegalMoves(GameState& gameState, Color color) {
	std::vector<Move> moves;
	std::vector<MoveInfo> history;
	generateAllMoves(gameState, moves, color, false);
	filterMoves(gameState, history, moves, color);

	std::string colorStr = (color == White) ? "White" : "Black";
	std::cout << "Legal Moves for " << colorStr << ":\n";
	for (const Move& move : moves) {
		int from = move.getStartSquare();
		int to   = move.getTargetSquare();

		char fromFile = 'a' + (from % 8);
		char fromRank = '1' + (from / 8);
		char toFile   = 'a' + (to % 8);
		char toRank   = '1' + (to / 8);

		std::cout << fromFile << fromRank << " -> " 
				  << toFile << toRank 
				  << " (flags=" << move.getFlags() << ")\n";
	}
}

void timePsuedoLegalPieceMoves(GameState& gameState, Piece piece) {
	std::vector<Move> moves;

	switch (piece) {
	case WPawn: {
		ScopedTimer timer("White pawn pseudo-legal move generation");
		generatePawnMoves(gameState, moves, White, false);
	} break;
	case BPawn: {
		ScopedTimer timer("Black pawn pseudo-legal move generation");
		generatePawnMoves(gameState, moves, Black, false);
	} break;
	case WKnight: {
		ScopedTimer timer("White knight pseudo-legal move generation");
		generateKnightMoves(gameState, moves, White, false);
	} break;
	case BKnight: {
		ScopedTimer timer("Black knight pseudo-legal move generation");
		generateKnightMoves(gameState, moves, Black, false);
	} break;
	case WBishop: {
		ScopedTimer timer("White bishop pseudo-legal move generation");
		generateBishopMoves(gameState, moves, White, false);
	} break;
	case BBishop: {
		ScopedTimer timer("Black bishop pseudo-legal move generation");
		generateBishopMoves(gameState, moves, Black, false);
	} break;
	case WRook: {
		ScopedTimer timer("White rook pseudo-legal move generation");
		generateRookMoves(gameState, moves, White, false);
	} break;
	case BRook: {
		ScopedTimer timer("Black rook pseudo-legal move generation");
		generateRookMoves(gameState, moves, Black, false);
	} break;
	case WQueen: {
		ScopedTimer timer("White queen pseudo-legal move generation");
		generateQueenMoves(gameState, moves, White, false);
	} break;
	case BQueen: {
		ScopedTimer timer("Black queen pseudo-legal move generation");
		generateQueenMoves(gameState, moves, Black, false);
	} break;
	case WKing: {
		ScopedTimer timer("White king pseudo-legal move generation");
		generateKingMoves(gameState, moves, White, false);
	} break;
	case BKing: {
		ScopedTimer timer("Black king pseudo-legal move generation");
		generateKingMoves(gameState, moves, Black, false);
	} break;
	default:
		std::cerr << "Invalid piece type passed to TimePsuedoLegalPieceMoves\n";
		return;
	}
}

void timeLegalPieceMoves(GameState& gameState, Piece piece) {
	std::vector<Move> moves;
	std::vector<MoveInfo> history;

	switch (piece) {
	case WPawn: {
		ScopedTimer timer("White pawn pseudo-legal move generation");
		generatePawnMoves(gameState, moves, White, false);
		filterMoves(gameState, history, moves, White);
	} break;
	case BPawn: {
		ScopedTimer timer("Black pawn pseudo-legal move generation");
		generatePawnMoves(gameState, moves, Black, false);
		filterMoves(gameState, history, moves, Black);
	} break;
	case WKnight: {
		ScopedTimer timer("White knight pseudo-legal move generation");
		generateKnightMoves(gameState, moves, White, false);
		filterMoves(gameState, history, moves, White);
	} break;
	case BKnight: {
		ScopedTimer timer("Black knight pseudo-legal move generation");
		generateKnightMoves(gameState, moves, Black, false);
		filterMoves(gameState, history, moves, Black);
	} break;
	case WBishop: {
		ScopedTimer timer("White bishop pseudo-legal move generation");
		generateBishopMoves(gameState, moves, White, false);
		filterMoves(gameState, history, moves, White);
	} break;
	case BBishop: {
		ScopedTimer timer("Black bishop pseudo-legal move generation");
		generateBishopMoves(gameState, moves, Black, false);
		filterMoves(gameState, history, moves, Black);
	} break;
	case WRook: {
		ScopedTimer timer("White rook pseudo-legal move generation");
		generateRookMoves(gameState, moves, White, false);
		filterMoves(gameState, history, moves, White);
	} break;
	case BRook: {
		ScopedTimer timer("Black rook pseudo-legal move generation");
		generateRookMoves(gameState, moves, Black, false);
		filterMoves(gameState, history, moves, Black);
	} break;
	case WQueen: {
		ScopedTimer timer("White queen pseudo-legal move generation");
		generateQueenMoves(gameState, moves, White, false);
		filterMoves(gameState, history, moves, White);
	} break;
	case BQueen: {
		ScopedTimer timer("Black queen pseudo-legal move generation");
		generateQueenMoves(gameState, moves, Black, false);
		filterMoves(gameState, history, moves, Black);
	} break;
	case WKing: {
		ScopedTimer timer("White king pseudo-legal move generation");
		generateKingMoves(gameState, moves, White, false);
		filterMoves(gameState, history, moves, White);
	} break;
	case BKing: {
		ScopedTimer timer("Black king pseudo-legal move generation");
		generateKingMoves(gameState, moves, Black, false);
		filterMoves(gameState, history, moves, Black);
	} break;
	default:
		std::cerr << "Invalid piece type passed to TimePsuedoLegalPieceMoves\n";
		return;
	}
}

void timePsuedoLegalMoves(GameState& gameState, Color color) {
	std::vector<Move> moves;

	ScopedTimer timer("Puesdo legal move generation"); 
	generateAllMoves(gameState, moves, color, false);
}

void timeLegalMoves(GameState& gameState, Color color) {
	std::vector<Move> moves;
	std::vector<MoveInfo> history;

	ScopedTimer timer("Legal move generation"); 
	generateAllMoves(gameState, moves, color, false);
	filterMoves(gameState, history, moves, color);
}

void timeMoveFiltering(GameState &gameState, Color color, std::vector<Move> moves) {
	if (moves.empty()) generateAllMoves(gameState, moves, color, false);
	std::vector<MoveInfo> history;

	ScopedTimer timer("Move filtering");
	filterMoves(gameState, history, moves, color);
}

void testPieceMoveGeneration(const std::string& fen, Piece piece, const std::string& expected) {
	GameState state(fen);
	std::vector<Move> moves;
	generatePieceMoves(state, moves, piece);

	std::cout << "--------------------------------------\n";
	std::cout << "EXPECTED: " << std::endl << expected << std::endl;
	std::cout << "RESULT: " << std::endl;
	for (auto& move : moves) {
		std::cout << move.moveToString() << " ";
	}
	std::cout << std::endl;
	std::cout << "--------------------------------------\n";
}

void testPawnMoveGeneration() {

	testPieceMoveGeneration("8/8/8/8/3p4/3P4/8/8 w - - 0 1", WPawn, "NO MOVES");
	testPieceMoveGeneration("8/8/8/8/3p4/3P4/8/8 b - - 0 1", BPawn, "NO MOVES");

	testPieceMoveGeneration("8/8/8/8/8/p7/7P/8 w - - 0 1", WPawn, "h2h3 h2h4");
	testPieceMoveGeneration("8/p7/7P/8/8/8/8/8 b - - 0 1", BPawn, "a7a6 a7a5");

	testPieceMoveGeneration("8/8/8/8/8/4p3/3P4/8 w - - 0 1", WPawn, "d2d3 d2d4 d2e3");
	testPieceMoveGeneration("8/3p4/4P3/8/8/8/8/8 b - - 0 1", BPawn, "d7d6 d7d5 d7e6");

	testPieceMoveGeneration("8/3pp3/4P3/8/8/8/8/8 w - - 0 1", WPawn, "e6d7");
	testPieceMoveGeneration("8/8/8/8/8/4p3/3PP3/8 b - - 0 1", BPawn, "e3d2");

	testPieceMoveGeneration("8/8/8/3pP3/8/8/8/8 w - d6 0 1", WPawn, "e5e6 e5d6");
	testPieceMoveGeneration("8/8/8/8/3Pp3/8/8/8 b - d3 0 1", BPawn, "e4e3 e4d3");

	testPieceMoveGeneration("8/8/8/3Pp3/8/8/8/8 w - e6 0 1", WPawn, "d5d6 d5e6");
	testPieceMoveGeneration("8/8/8/8/3pP3/8/8/8 b - e3 0 1", BPawn, "d4d3 d4e3");

	testPieceMoveGeneration("3qp3/4P3/8/8/8/8/8/8 w - - 0 1", WPawn, "e7d8q e7d8n e7d8r e7d8b");
	testPieceMoveGeneration("8/8/8/8/8/8/4p3/3QP3 b - - 0 1", BPawn, "e2d1q e2d1n e2d1r e2d1b");
	
	testPieceMoveGeneration("3q4/4P3/8/8/8/8/8/8 w - - 0 1", WPawn, "e7e8q e7e8n e7e8r e7e8b e7d8q e7d8n e7d8r e7d8b");
	testPieceMoveGeneration("8/8/8/8/8/8/4p3/3Q4 b - - 0 1", BPawn, "e2e1q e2e1n e2e1r e2e1b e2d1q e2d1n e2d1r e2d1b");

}

void testKnightMoveGeneration() {

	testPieceMoveGeneration("8/8/8/8/8/8/6N1/8 w - - 0 1", WKnight, "g2e3 g2f4 g2h4 g2e1");
	testPieceMoveGeneration("8/8/8/8/8/8/6n1/8 b - - 0 1", BKnight, "g2e3 g2f4 g2h4 g2e1");

	testPieceMoveGeneration("8/8/8/8/8/8/1N6/8 w - - 0 1", WKnight, "b2d3 b2a4 b2c4 b2d1");
	testPieceMoveGeneration("8/8/8/8/8/8/1n6/8 b - - 0 1", BKnight, "b2d3 b2a4 b2c4 b2d1");

	testPieceMoveGeneration("8/8/3p4/8/4N3/2q5/5r2/8 w - - 0 1", WKnight, "e4d6 e4c3 e4f2 e4c5 e4g5 e4f6 e4g3 e4d");
	testPieceMoveGeneration("8/8/3P4/8/4n3/2Q5/5R2/8 b - - 0 1", BKnight, "e4d6 e4c3 e4f2 e4c5 e4g5 e4f6 e4g3 e4d");

	testPieceMoveGeneration("8/8/3P4/8/4N3/2Q5/5R2/8 w - - 0 1", WKnight, "e4c5 e4g5 e4f6 e4g3 e4d2");
	testPieceMoveGeneration("8/8/3p4/8/4n3/2q5/5r2/8 b - - 0 1", BKnight, "e4c5 e4g5 e4f6 e4g3 e4d2");
}

void testBishopMoveGeneration() {
	testPieceMoveGeneration("8/8/8/8/p3p3/8/6B1/5R2 w - - 0 1", WBishop, "g2f3 g2e4 g2h3 g2h1");
	testPieceMoveGeneration("8/8/8/8/P3P3/8/6b1/5r2 b - - 0 1", BBishop, "g2f3 g2e4 g2h3 g2h1");
	
	testPieceMoveGeneration("8/8/8/3Q1P2/4B3/3P1R2/8/8 b - - 0 1", WBishop, "NO MOVES");
	testPieceMoveGeneration("8/8/8/3q1p2/4b3/3p1r2/8/8 b - - 0 1", BBishop, "NO MOVES");

	testPieceMoveGeneration("8/1p5P/2R5/8/4B3/8/6p1/1P6 w - - 0 1", WBishop, "e4d5 e4f5 e4g6 e4f3 e4g2 e4d3 e4c2");
	testPieceMoveGeneration("8/1P5p/2r5/8/4b3/8/6P1/1p6 b - - 0 1", BBishop, "e4d5 e4f5 e4g6 e4f3 e4g2 e4d3 e4c2");

	testPieceMoveGeneration("8/8/2R5/3p1n2/4B3/3p1p2/8/8 w - - 0 1", WBishop, "e4d5 e4f5 e4f3 e4d3");
	testPieceMoveGeneration("8/8/2r5/3P1N2/4b3/3P1P2/8/8 b - - 0 1", BBishop, "e4d5 e4f5 e4f3 e4d3");

}

void testRookMoveGeneration() {
	testPieceMoveGeneration("4P3/8/8/4R1P1/8/8/8/8 w - - 0 1", WRook, "e5f5 e5e6 e5e7 e5d5 e5c5 e5b5 e5a5 e5e4 e5e3 e5e2 e5e1");
	testPieceMoveGeneration("4p3/8/8/4r1p1/8/8/8/8 b - - 0 1", BRook, "e5f5 e5e6 e5e7 e5d5 e5c5 e5b5 e5a5 e5e4 e5e3 e5e2 e5e1");

	testPieceMoveGeneration("8/p7/3R3P/8/8/3P4/3p4/8 w - - 0 1", WRook, "d6e6 d6f6 d6g6 d6d7 d6d8 d6c6 d6b6 d6a6 d6d5 d6d4");
	testPieceMoveGeneration("8/P7/3r3p/8/8/3p4/3P4/8 b - - 0 1", BRook, "d6e6 d6f6 d6g6 d6d7 d6d8 d6c6 d6b6 d6a6 d6d5 d6d4");

	testPieceMoveGeneration("8/8/8/8/8/8/8/2p4R w - - 0 1", WRook, "h1h2 h1h3 h1h4 h1h5 h1h6 h1h7 h1h8 h1g1 h1f1 h1e1 h1d1 h1c1");
	testPieceMoveGeneration("8/8/8/8/8/8/8/2P4r b - - 0 1", BRook, "h1h2 h1h3 h1h4 h1h5 h1h6 h1h7 h1h8 h1g1 h1f1 h1e1 h1d1 h1c1");

	testPieceMoveGeneration("R7/8/8/8/8/8/8/8 w - - 0 1", WRook, "a8b8 a8c8 a8d8 a8e8 a8f8 a8g8 a8h8 a8a7 a8a6 a8a5 a8a4 a8a3 a8a2 a8a1");
	testPieceMoveGeneration("r7/8/8/8/8/8/8/8 w - - 0 1", BRook, "a8b8 a8c8 a8d8 a8e8 a8f8 a8g8 a8h8 a8a7 a8a6 a8a5 a8a4 a8a3 a8a2 a8a1");
}

void testQueenMoveGeneration() {
	testPieceMoveGeneration("8/8/3brb2/3qQp2/3ppp2/8/8/8 w - - 0 1", WQueen, "e5f5 e5e6 e5d5 e5e4 e5d6 e5f6 e5f4 e5d4");
	testPieceMoveGeneration("8/8/3BRB2/3QqP2/3PPP2/8/8/8 b - - 0 1", BQueen, "e5f5 e5e6 e5d5 e5e4 e5d6 e5f6 e5f4 e5d4");

	testPieceMoveGeneration("8/8/3PPP2/3PQB2/3NRB2/8/8/8 w - - 0 1", WQueen, "NO MOVES");
	testPieceMoveGeneration("8/8/3ppp2/3pqb2/3nrb2/8/8/8 b - - 0 1", BQueen, "NO MOVES");

	testPieceMoveGeneration("8/8/8/8/8/8/8/7Q w - - 0 1", WQueen, "h1h2 h1h3 h1h4 h1h5 h1h6 h1h7 h1h8 h1g1 h1f1 h1e1 h1d1 h1c1 h1b1 h1a1 h1g2 h1f3 h1e4 h1d5 h1c6 h1b7 h1a8");
	testPieceMoveGeneration("8/8/8/8/8/8/8/7q b - - 0 1", BQueen, "h1h2 h1h3 h1h4 h1h5 h1h6 h1h7 h1h8 h1g1 h1f1 h1e1 h1d1 h1c1 h1b1 h1a1 h1g2 h1f3 h1e4 h1d5 h1c6 h1b7 h1a8");

	testPieceMoveGeneration("8/4P3/3p4/8/8/Q3Pp2/8/8 w - - 0 1", WQueen, "a3b3 a3c3 a3d3 a3a4 a3a5 a3a6 a3a7 a3a8 a3a2 a3a1 a3b4 a3c5 a3d6 a3b2 a3c1");
	testPieceMoveGeneration("8/4p3/3P4/8/8/q3pP2/8/8 b - - 0 1", BQueen, "a3b3 a3c3 a3d3 a3a4 a3a5 a3a6 a3a7 a3a8 a3a2 a3a1 a3b4 a3c5 a3d6 a3b2 a3c1");
}

void testKingMoveGeneration() {
	testPieceMoveGeneration("8/8/8/8/8/8/8/7K w - - 0 1", WKing, "h1h2 h1g1 h1g2");
	testPieceMoveGeneration("8/8/8/8/8/8/8/7k b - - 0 1", BKing, "h1h2 h1g1 h1g2");

	testPieceMoveGeneration("8/8/8/8/8/5B1n/5BK1/6p1 w - - 0 1", WKing, "g2h2 g2g3 g2g1 g2h3 g2h1 g2f1");
	testPieceMoveGeneration("8/8/8/8/8/5b1N/5bk1/6P1 b - - 0 1", BKing, "g2h2 g2g3 g2g1 g2h3 g2h1 g2f1");

	testPieceMoveGeneration("8/8/8/8/8/8/8/4K2R w K - 0 1", WKing, "e1f1 e1e2 e1d1 e1d2 e1f2 e1g1");
	testPieceMoveGeneration("4k2r/8/8/8/8/8/8/8 b k - 0 1", BKing, "e8f8 e8d8 e8e7 e8f7 e8d7 e8g8");

	testPieceMoveGeneration("8/8/8/8/8/8/8/R3K3 w Q - 0 1", WKing, "e1f1 e1e2 e1d1 e1d2 e1f2 e1c1");
	testPieceMoveGeneration("r3k3/8/8/8/8/8/8/8 b q - 0 1", BKing, "e8f8 e8d8 e8e7 e8f7 e8d7 e8c8");

	testPieceMoveGeneration("8/8/8/8/8/8/5n2/R1R1K3 w Q - 1 1", WKing, "e1f1 e1e2 e1d1 e1d2 e1f2");
	testPieceMoveGeneration("r1r1k3/5N2/8/8/8/8/8/8 b q - 1 1", BKing, "e8f8 e8d8 e8e7 e8f7 e8d7");

	testPieceMoveGeneration("8/8/8/8/8/8/8/R3K2R w KQ - 1 1", WKing, "e1f1 e1e2 e1d1 e1d2 e1f2 e1g1 e1c1");
	testPieceMoveGeneration("r3k2r/8/8/8/8/8/8/8 b kq - 1 1", BKing, "e8f8 e8d8 e8e7 e8f7 e8d7 e8g8 e8c8");
}

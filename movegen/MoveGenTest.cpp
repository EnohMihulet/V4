#include <chrono>
#include <iostream>
#include <vector>

#include "MoveGen.h"
#include "MoveGenTest.h"
#include "../helpers/timer.h"


void printPieceMoves(GameState& gameState, Piece piece) {
	std::vector<Move> moves;
	std::vector<MoveInfo> history;

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

	// filterMoves(gameState, history, moves, color);

	// std::cout << "Legal Moves for piece " << (int)piece << ":\n";
	// for (const Move& move : moves) {
	// 	int from = move.getStartSquare();
	// 	int to   = move.getTargetSquare();

	// 	char fromFile = 'a' + (from % 8);
	// 	char fromRank = '1' + (from / 8);
	// 	char toFile   = 'a' + (to % 8);
	// 	char toRank   = '1' + (to / 8);

	// 	std::cout << fromFile << fromRank << " -> " 
	// 			  << toFile << toRank 
	// 			  << " (flags=" << move.getFlags() << ")\n";
	// }
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

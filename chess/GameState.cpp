#include <array>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#include "GameState.h"
#include "../helpers/GameStateHelper.h"
#include "Common.h"
#include "../helpers/Zobrist.h"


Piece charToPiece(char c); 
uint16 squareCharToInt(char c); 
GameState::GameState() {
	board.fill(EMPTY);
	bitboards.fill(0ULL);
	zobristHash = 0ULL;
	castlingRights = 0;
	enPassantFile = NO_ENPASSANT_FILE;
	halfMoves = 0;
	fullMoves = 1;
	colorToMove = White;
}

GameState::GameState(const std::string& fen) {
	setPosition(fen);
}

void GameState::setPosition(const std::string& fen) {
	board.fill(EMPTY);
	bitboards.fill(0ULL);
	zobristHash = 0ULL;
	castlingRights = 0;
	enPassantFile = NO_ENPASSANT_FILE;
	halfMoves = 0;
	fullMoves = 1;
	colorToMove = White;

	std::istringstream ss(fen);
	std::string boardPart, activeColor, castling, enPassant, halfMoveStr, fullMoveStr;
	ss >> boardPart >> activeColor >> castling >> enPassant >> halfMoveStr >> fullMoveStr;

	uint16 rank = 7;
	uint16 file = 0;

	for (char c : boardPart) {
		if (c == '/') {
			rank--;
			file = 0;
			continue;
		}
		else if (std::isdigit(static_cast<unsigned char>(c))) {
			file += c - '0';
			continue;
		}
		else {
			Piece piece = charToPiece(c);
			uint16 square = rank * 8 + file;
			uint64 squareBit = 1ULL << square;

			zobristHash ^= PIECE_ZOBRIST_KEYS[64*piece + square];

			bitboards[piece] |= squareBit;
			bitboards[AllIndex] |= squareBit;
			board[square] = piece;

			if (isWhite(piece)) bitboards[WhiteIndex] |= squareBit;
			else bitboards[BlackIndex] |= squareBit;

			file++;
		}
	}

	if (!activeColor.empty() && activeColor[0] == 'b') {
		zobristHash ^= BLACK_ZOBRIST_KEY;
		colorToMove = Black;
	}
	else colorToMove = White;

	castlingRights = 0;
	if (castling != "-") {
		for (char c : castling) {
			switch (c) {
			case 'K': castlingRights |= W_KING_SIDE; break;
			case 'Q': castlingRights |= W_QUEEN_SIDE; break;
			case 'k': castlingRights |= B_KING_SIDE; break;
			case 'q': castlingRights |= B_QUEEN_SIDE; break;
			default: break;
			}
		}
	}
	zobristHash ^= CASTLING_ZOBRIST_KEYS[castlingRights];

	enPassantFile = NO_ENPASSANT_FILE;
	if (enPassant != "-") {
		enPassantFile = squareCharToInt(enPassant[0]);
		if (isEnPassantCaptureLegal(enPassantFile, colorToMove)) {
			zobristHash ^= ENPASSANT_ZOBRIST_KEYS[enPassantFile];
		}
	}

	if (!halfMoveStr.empty()) halfMoves = static_cast<uint8>(std::stoi(halfMoveStr));
	else halfMoves = 0;

	if (!fullMoveStr.empty()) fullMoves = static_cast<uint8>(std::stoi(fullMoveStr));
	else fullMoves = 1;
}

void GameState::makeMove(Move move, std::vector<MoveInfo>& history) {
	uint16 targetSq = move.getTargetSquare();
	uint16 startSq = move.getStartSquare();
	assert(targetSq <= 63 && startSq <= 63);
	if (pieceAt(startSq) == EMPTY) std::cout << move.moveToString() << std::endl;
	assert(pieceAt(startSq) != EMPTY);

	MoveInfo moveInfo;
	moveInfo.halfMoves = halfMoves;
	moveInfo.castlingRights = castlingRights;
	moveInfo.enPassantFile = enPassantFile;
	moveInfo.zobristHash = zobristHash;
	moveInfo.capturedPiece = pieceAt(targetSq);
	#ifdef DEBUG_MODE
	moveInfo.bitboards = bitboards;
	#endif

	Piece piece = pieceAt(startSq);
	bool iswhite = isWhite(piece);
	uint16 flags = move.getFlags();

	clearSquare(startSq);

	zobristHash ^= PIECE_ZOBRIST_KEYS[64*piece + startSq];
	zobristHash ^= CASTLING_ZOBRIST_KEYS[castlingRights];
	if (enPassantFile != NO_ENPASSANT_FILE) zobristHash ^= ENPASSANT_ZOBRIST_KEYS[enPassantFile];
	enPassantFile = NO_ENPASSANT_FILE;

	if (IS_SIMPLE_MOVE[flags]) [[likely]] {
		if (move.isCapture()) {
			halfMoves = 0;
			zobristHash ^= PIECE_ZOBRIST_KEYS[64*pieceAt(targetSq) + targetSq];
		}
		else if (piece == WPawn || piece == BPawn) halfMoves = 0;
		else halfMoves++;

		if (move.isTwoUpMove()) {
			int16 file = targetSq & 7;
			if (isEnPassantCaptureLegal(file, colorToMove == White ? Black : White)) {
				zobristHash ^= ENPASSANT_ZOBRIST_KEYS[file];
			}
			enPassantFile = file;
		}
		
		castlingRights &= CASTLING_RIGHTS_MASK[startSq];
		castlingRights &= CASTLING_RIGHTS_MASK[targetSq];
		
		clearSquare(targetSq);
		setPiece(targetSq, piece);

		zobristHash ^= CASTLING_ZOBRIST_KEYS[castlingRights];
		zobristHash ^= PIECE_ZOBRIST_KEYS[64*piece + targetSq];
	}
	else {
		halfMoves = 0;

		switch (flags) { 
		case (EN_PASSANT_FLAG): {
			uint16 captureSq = iswhite ? targetSq - 8 : targetSq + 8;
			moveInfo.capturedPiece = pieceAt(captureSq);

			setPiece(targetSq, piece);
			clearSquare(captureSq);
			zobristHash ^= PIECE_ZOBRIST_KEYS[64*moveInfo.capturedPiece + captureSq];
			zobristHash ^= PIECE_ZOBRIST_KEYS[64*piece + targetSq];
		} break;
		case (KING_SIDE_FLAG): {
			setPiece(targetSq, piece);
			zobristHash ^= PIECE_ZOBRIST_KEYS[64*piece + targetSq];
			iswhite ? makeWKingSide(*this) : makeBKingSide(*this); 
		} break;
		case (QUEEN_SIDE_FLAG): {
			setPiece(targetSq, piece);
			zobristHash ^= PIECE_ZOBRIST_KEYS[64*piece + targetSq];
			iswhite ? makeWQueenSide(*this) : makeBQueenSide(*this); 
		} break;
		case (QUEEN_PROMOTE_CAPTURE): zobristHash ^= PIECE_ZOBRIST_KEYS[64*pieceAt(targetSq) + targetSq];
		case (QUEEN_PROMOTE_FLAG): {
			clearSquare(targetSq);
			if (iswhite) {
				setPiece(targetSq, WQueen);
				zobristHash ^= PIECE_ZOBRIST_KEYS[WQueen*64 + targetSq];
			}
			else {
				setPiece(targetSq, BQueen);
				zobristHash ^= PIECE_ZOBRIST_KEYS[BQueen*64 + targetSq];
			}
		} break;
		case (KNIGHT_PROMOTE_CAPTURE): zobristHash ^= PIECE_ZOBRIST_KEYS[64*pieceAt(targetSq) + targetSq];
		case (KNIGHT_PROMOTE_FLAG): {
			clearSquare(targetSq);
			if (iswhite) {
				setPiece(targetSq, WKnight);
				zobristHash ^= PIECE_ZOBRIST_KEYS[WKnight*64 + targetSq];
			}
			else {
				setPiece(targetSq, BKnight);
				zobristHash ^= PIECE_ZOBRIST_KEYS[BKnight*64 + targetSq];
			}
		} break;
		case (ROOK_PROMOTE_CAPTURE): zobristHash ^= PIECE_ZOBRIST_KEYS[64*pieceAt(targetSq) + targetSq];
		case (ROOK_PROMOTE_FLAG): {
			clearSquare(targetSq);
			if (iswhite) {
				setPiece(targetSq, WRook);
				zobristHash ^= PIECE_ZOBRIST_KEYS[WRook*64 + targetSq];
			}
			else {
				setPiece(targetSq, BRook);
				zobristHash ^= PIECE_ZOBRIST_KEYS[BRook*64 + targetSq];
			}
		} break;
		case (BISHOP_PROMOTE_CAPTURE): zobristHash ^= PIECE_ZOBRIST_KEYS[64*pieceAt(targetSq) + targetSq]; 
		case (BISHOP_PROMOTE_FLAG): {
			clearSquare(targetSq);
			if (iswhite) {
				setPiece(targetSq, WBishop);
				zobristHash ^= PIECE_ZOBRIST_KEYS[WBishop*64 + targetSq];
			}
			else {
				setPiece(targetSq, BBishop);
				zobristHash ^= PIECE_ZOBRIST_KEYS[BBishop*64 + targetSq];
			}
		} break;
		default: break;
		}
		zobristHash ^= CASTLING_ZOBRIST_KEYS[castlingRights];
	};

	zobristHash ^= BLACK_ZOBRIST_KEY;
	if (colorToMove == White) colorToMove = Black;
	else {
		colorToMove = White;
		fullMoves++;
	}
	history.push_back(moveInfo);
}

void GameState::unmakeMove(Move move, std::vector<MoveInfo>& history) {
	assert(!history.empty());
	MoveInfo moveInfo = history.back(); 
	history.pop_back();

	zobristHash = moveInfo.zobristHash;
	halfMoves = moveInfo.halfMoves;
	castlingRights = moveInfo.castlingRights;
	enPassantFile = moveInfo.enPassantFile;


	uint16 targetSq = move.getTargetSquare();
	uint16 startSq = move.getStartSquare();
	assert(targetSq <= 63 && startSq <= 63);

	Piece piece = pieceAt(targetSq);
	bool iswhite = isWhite(piece);
	uint16 flags = move.getFlags();
	
	if (IS_SIMPLE_MOVE[flags]) [[likely]] {
		clearSquare(targetSq);
		if (moveInfo.capturedPiece != EMPTY) setPiece(targetSq, moveInfo.capturedPiece);
		setPiece(startSq, piece);
	} 
	else {
		switch(flags) {
		case (EN_PASSANT_FLAG): {
			uint16 captureSq = iswhite ? targetSq - 8 : targetSq + 8;
			clearSquare(targetSq);
			setPiece(startSq, piece);
			setPiece(captureSq, moveInfo.capturedPiece);
		} break;
		case (KING_SIDE_FLAG): {
			iswhite ? undoWKingSide(*this) : undoBKingSide(*this);
		} break;
		case (QUEEN_SIDE_FLAG): {
			iswhite ? undoWQueenSide(*this): undoBQueenSide(*this);
		} break;
		case (QUEEN_PROMOTE_FLAG): case (QUEEN_PROMOTE_CAPTURE): case (KNIGHT_PROMOTE_FLAG): case (KNIGHT_PROMOTE_CAPTURE): 
		case (ROOK_PROMOTE_FLAG): case (ROOK_PROMOTE_CAPTURE): case (BISHOP_PROMOTE_FLAG): case (BISHOP_PROMOTE_CAPTURE): {
			clearSquare(targetSq);
			if (moveInfo.capturedPiece != EMPTY) setPiece(targetSq, moveInfo.capturedPiece);
			setPiece(startSq, iswhite ? WPawn : BPawn);
		} break;
		default: break;
		}
	}
	if (colorToMove == White) {
		colorToMove = Black;
		fullMoves--;
	}
	else colorToMove = White;

	#ifdef DEBUG_MODE
	// for (size_t i = 0; i < bitboards.size(); i++) {
	// 	if (moveInfo.bitboards[i] != bitboards[i]) {
	// 		std::cout << (move.moveToString()) << std::endl;
	// 		printBitboard(moveInfo.bitboards[i]);
	// 		printBitboard(bitboards[i]);
	// 	}
	// }
	#endif
}

void GameState::setPiece(uint16 square, Piece piece) {
	board[square] = piece;
	Bitboard bb = (1ULL << square);
	bitboards[piece] |= bb;
	bitboards[AllIndex] |= bb;
	isWhite(piece) ? bitboards[WhiteIndex] |= bb : bitboards[BlackIndex] |= bb;
}

void GameState::clearSquare(uint16 square) {
	Piece p = board[square];
	Bitboard bb = (1ULL << square);

	if (p == EMPTY) return;

	board[square] = EMPTY;
	bitboards[p] ^= bb;
	bitboards[AllIndex] ^= bb;
	isWhite(p) ? bitboards[WhiteIndex] ^= bb : bitboards[BlackIndex] ^= bb;
}

Piece GameState::pieceAt(uint16 sq) const {
	return board[sq];
}

bool GameState::isEnPassantCaptureLegal(uint16 enPassantFile, Color color) const {
	if (enPassantFile >= 8) return false;

	const bool whiteToMove = (color == White);
	const Bitboard pawns = bitboards[whiteToMove ? WPawn : BPawn];

	const uint16 epRank = whiteToMove ? 5 : 2;
	const Bitboard epTargetSq = 1ULL << (epRank * 8 + enPassantFile);

	Bitboard potentialAttackers;
	if (whiteToMove) potentialAttackers = ((epTargetSq >> 7) & ~FILE_H) | ((epTargetSq >> 9) & ~FILE_A);
	else potentialAttackers = ((epTargetSq << 7) & ~FILE_A) | ((epTargetSq << 9) & ~FILE_H);

	return (pawns & potentialAttackers) != 0ULL;
}


std::string GameState::toFenString() {
	std::ostringstream fen;
	for (int rank = 7; rank >= 0; --rank) {
		int emptyCount = 0;
		for (int file = 0; file < 8; ++file) {
			const int sq = rank * 8 + file;
			const Piece p = board[sq];
			const char c = pieceToChar(p);
			if (c == '\0') {
				++emptyCount;
			} else {
				if (emptyCount) { fen << emptyCount; emptyCount = 0; }
				fen << c;
			}
		}
		if (emptyCount) fen << emptyCount;
		if (rank) fen << '/';
	}

	fen << ' ' << (colorToMove == Color::White ? 'w' : 'b') << ' ';

	bool anyCastle = false;
	if (castlingRights & W_KING_SIDE) { fen << 'K'; anyCastle = true; }
	if (castlingRights & W_QUEEN_SIDE) { fen << 'Q'; anyCastle = true; }
	if (castlingRights & B_KING_SIDE) { fen << 'k'; anyCastle = true; }
	if (castlingRights & B_QUEEN_SIDE) { fen << 'q'; anyCastle = true; }
	if (!anyCastle) fen << '-';
	fen << ' ';

	if (enPassantFile < 8 && isEnPassantCaptureLegal(enPassantFile, colorToMove)) {
		const char fileChar = static_cast<char>('a' + enPassantFile);
		const int rankNum = (colorToMove == Color::White) ? 6 : 3;
		fen << fileChar << rankNum;
	} else {
		fen << '-';
	}

	fen << ' ' << static_cast<unsigned>(halfMoves) << ' ' << static_cast<unsigned>(fullMoves);

	return fen.str();
}

#include <array>
#include <iostream>
#include <string>

#include "GameState.h"


Piece charToPiece(char c); 
uint16 squareCharToInt(char c); 

GameState::GameState(const std::string& fen) {
	board.fill(EMPTY);
	bitboards.fill(0ULL);
	zobristHash = 0ULL;
	castlingRights = 0;
	enPassantFile = NO_ENPASSANT_FILE;
	halfMoves = 0;
	fullMoves = 1;
	colorToMove = White;

	uint16 rank = 7;
	uint16 file = 0;
	uint16 square = 0;
	for (std::size_t i = 0; i < fen.size(); i++) {
		char c = fen[i];
		if (c == ' ') {
			c = fen[++i];
			if (c == 'w') colorToMove = White;
			else colorToMove = Black;
			i += 2;
			while (fen[i] != ' ') {
				c = fen[i];
				if (c == '-') castlingRights = 0;
				else if (c == 'K') castlingRights |= W_KING_SIDE;
				else if (c == 'Q') castlingRights |= W_QUEEN_SIDE;
				else if (c == 'k') castlingRights |= B_KING_SIDE;
				else if (c == 'q') castlingRights |= B_QUEEN_SIDE;
				i++;
			}
			c = fen[++i];
			if (c == '-') enPassantFile = NO_ENPASSANT_FILE;
			else {
				enPassantFile = squareCharToInt(c);
				i += 3;
			}
				while (fen[i] != ' ') {
				c = fen[i];
				halfMoves = halfMoves * 10 + (c - '0');
				i++;
			}
			i++;
			while (i < fen.size()) {
				c = fen[i];
				fullMoves = fullMoves * 10 + (c - '0');
				i++;
			}
			return;
		}
		else if (c == '/') {
			file = 0;
			rank--;
			continue;
		}
		
		uint16 intVal = c - '0'; 
		if (intVal >= 1 && intVal <= 8) {
			file += intVal;
		}
		else {
			Piece piece = charToPiece(c);
			square = rank * 8 + file;
			uint64 squareBit = 1ULL << square;
			bitboards[piece] |= squareBit;
			bitboards[AllIndex] |= squareBit;
			board[square] = piece;
			isWhite(piece) ? bitboards[WhiteIndex] |= squareBit : bitboards[BlackIndex] |= squareBit;
			file++;
		}
		
	}
	return;
}

void GameState::makeMove(Move move, std::vector<MoveInfo>& history) {
	// TODO UPDATE ZOBRIST
	uint16 targetSq = move.getTargetSquare();
	uint16 startSq = move.getStartSquare();
	assert(targetSq <= 63 && startSq <= 63);
	assert(pieceAt(startSq) != EMPTY);

	MoveInfo moveInfo;
	moveInfo.halfMoves = halfMoves;
	moveInfo.castlingRights = castlingRights;
	moveInfo.enPassantFile = enPassantFile;
	moveInfo.zobristHash = zobristHash;
	moveInfo.capturedPiece = pieceAt(targetSq);

	Piece piece = pieceAt(startSq);

	clearSquare(startSq);
	if (move.isEnPassant()) {
		uint16 captureSq = isWhite(piece) ? targetSq - 8 : targetSq + 8;
		moveInfo.capturedPiece = pieceAt(captureSq); // Is this or checking piece color and setting it to respective pawn

		setPiece(targetSq, piece);
		clearSquare(captureSq);
		halfMoves = 0;
		enPassantFile = NO_ENPASSANT_FILE;
	}
	else if (move.isKingSideCastle()) {
		setPiece(targetSq, piece);
		if (isWhite(piece)) {
			castlingRights &= (B_KING_SIDE | B_QUEEN_SIDE);
			clearSquare(7);
			setPiece(5, WRook);
		}
		else {
			castlingRights &= (W_KING_SIDE | W_QUEEN_SIDE);
			clearSquare(63);
			setPiece(61, BRook);
		}
		halfMoves = 0;
		enPassantFile = NO_ENPASSANT_FILE;
	}
	else if (move.isQueenSideCastle()) {
		setPiece(targetSq, piece);
		if (isWhite(piece)) {
			castlingRights &= (B_KING_SIDE | B_QUEEN_SIDE);
			clearSquare(0);
			setPiece(3, WRook);
		}
		else {
			castlingRights &= (W_KING_SIDE | W_QUEEN_SIDE);
			clearSquare(56);
			setPiece(59, BRook);
		}
		halfMoves = 0;
		enPassantFile = NO_ENPASSANT_FILE;
	}
	else if (move.isPromotion()) {
		clearSquare(targetSq);
		if (move.isQueenPromotion()) isWhite(piece) ? setPiece(targetSq, WQueen) : setPiece(targetSq, BQueen); 
		else if (move.isKnightPromotion()) isWhite(piece) ? setPiece(targetSq, WKnight) : setPiece(targetSq, BKnight); 
		else if (move.isRookPromotion()) isWhite(piece) ? setPiece(targetSq, WRook) : setPiece(targetSq, BRook); 
		else if (move.isBishopPromotion()) isWhite(piece) ? setPiece(targetSq, WBishop) : setPiece(targetSq, BBishop); 
		halfMoves = 0;
		enPassantFile = NO_ENPASSANT_FILE;
	}
	else {
		if (move.isCapture() || piece == WPawn || piece == BPawn) halfMoves = 0;
		else halfMoves++;

		if (move.isTwoUpMove()) enPassantFile = targetSq % 8;
		else enPassantFile = NO_ENPASSANT_FILE;

		if (piece == WKing && startSq == 4) castlingRights &= (B_KING_SIDE | B_QUEEN_SIDE);
		else if ((targetSq == 0 && pieceAt(targetSq) == WRook) || (piece == WRook && startSq == 0)) castlingRights &= (B_KING_SIDE | B_QUEEN_SIDE | W_KING_SIDE);
		else if ((targetSq == 7 && pieceAt(targetSq) == WRook) || (piece == WRook && startSq == 7)) castlingRights &= (B_KING_SIDE | B_QUEEN_SIDE | W_QUEEN_SIDE);
		else if (piece == BKing && startSq == 60) castlingRights &= (W_KING_SIDE | W_QUEEN_SIDE); 
		else if ((targetSq == 56 && pieceAt(targetSq) == BRook) || (piece == BRook && startSq == 56)) castlingRights &= (W_KING_SIDE | W_QUEEN_SIDE | B_KING_SIDE);
		else if ((targetSq == 63 && pieceAt(targetSq) == BRook) || (piece == BRook && startSq == 63)) castlingRights &= (W_KING_SIDE | W_QUEEN_SIDE | B_KING_SIDE);
		
		clearSquare(targetSq);
		setPiece(targetSq, piece);
	}
	colorToMove = colorToMove == White ? Black : White;
	if (colorToMove == White) fullMoves++;
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
	
	if (move.isEnPassant()) {
		uint16 captureSq = isWhite(piece) ? targetSq - 8 : targetSq + 8;
		clearSquare(targetSq);
		setPiece(startSq, piece);
		setPiece(captureSq, moveInfo.capturedPiece);
	}
	else if (move.isKingSideCastle()) {
		if (isWhite(piece)) {
			clearSquare(5);
			setPiece(7, WRook);
			clearSquare(6);
			setPiece(4, WKing);
		}
		else {
			clearSquare(61);
			setPiece(63, BRook);
			clearSquare(62);
			setPiece(60, BKing);
		}
	}
	else if (move.isQueenSideCastle()) {
		if (isWhite(piece)) {
			clearSquare(3);
			setPiece(0, WRook);
			clearSquare(2);
			setPiece(4, WKing);
		}
		else {
			clearSquare(59);
			setPiece(56, BRook);
			clearSquare(58);
			setPiece(60, BKing);
		}
	}
	else if (move.isPromotion()) {
		clearSquare(targetSq);
		if (moveInfo.capturedPiece != EMPTY) setPiece(targetSq, moveInfo.capturedPiece);
		setPiece(startSq, isWhite(piece) ? WPawn : BPawn);
	}
	else {
		clearSquare(targetSq);
		if (moveInfo.capturedPiece != EMPTY) setPiece(targetSq, moveInfo.capturedPiece);
		setPiece(startSq, piece);
	}
	if (colorToMove == White) {
		fullMoves--;
		colorToMove = Black;
	}
	else {
		colorToMove = White;
	}
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

Piece charToPiece(char c) {
	switch (c) {
	case 'p': return BPawn;
	case 'n': return BKnight;
	case 'b': return BBishop;
	case 'r': return BRook;
	case 'q': return BQueen;
	case 'k': return BKing;
	case 'P': return WPawn;
	case 'N': return WKnight;
	case 'B': return WBishop;
	case 'R': return WRook;
	case 'Q': return WQueen;
	case 'K': return WKing;
	}
	std::cerr << "Invalid char to conver to piece: " << c << std::endl;
	return WPawn;
}

uint16 squareCharToInt(char c) {
	switch (c) {
		case 'a': return 0;
		case 'b': return 1;
		case 'c': return 2;
		case 'd': return 3;
		case 'e': return 4;
		case 'f': return 5;
		case 'g': return 6;
		case 'h': return 7;
	}
	int16 x = c - '0';
	if (x >= 0 && x < 8) {
		return x;
	}
	std::cerr << "Invalid file character: " << c << std::endl;
	return 0;
}

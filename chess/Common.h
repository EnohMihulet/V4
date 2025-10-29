#pragma once
#include <cassert>
#include <bit>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string_view>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint8_t Piece;
typedef uint64_t Bitboard;

constexpr uint16 NO_FLAG = 	          0b0000;
constexpr uint16 CAPTURE_FLAG =           0b0001;
constexpr uint16 EN_PASSANT_FLAG =        0b0011;
constexpr uint16 PAWN_TWO_UP_FLAG  =      0b0010;
constexpr uint16 KING_SIDE_FLAG =         0b0100;
constexpr uint16 QUEEN_SIDE_FLAG =	  0b0110;

constexpr uint16 QUEEN_PROMOTE_FLAG =     0b1110;
constexpr uint16 KNIGHT_PROMOTE_FLAG =    0b1000;
constexpr uint16 BISHOP_PROMOTE_FLAG =	  0b1010;
constexpr uint16 ROOK_PROMOTE_FLAG =	  0b1100;
constexpr uint16 QUEEN_PROMOTE_CAPTURE =  0b1111;
constexpr uint16 KNIGHT_PROMOTE_CAPTURE = 0b1001;
constexpr uint16 BISHOP_PROMOTE_CAPTURE = 0b1011;
constexpr uint16 ROOK_PROMOTE_CAPTURE =   0b1101;

constexpr uint16 START_SQUARE_MASK =   0b0000000000111111;
constexpr uint16 TARGET_SQUARE_MASK =  0b0000111111000000;
constexpr uint16 FLAG_MASK =	       0b1111000000000000;

constexpr uint16 W_KING_SIDE =  0b0001;
constexpr uint16 W_QUEEN_SIDE = 0b0010;
constexpr uint16 B_KING_SIDE =  0b0100;
constexpr uint16 B_QUEEN_SIDE = 0b1000;

constexpr uint16 NO_ENPASSANT_FILE = 8;

constexpr uint64 FILE_A = 0x0101010101010101ULL;
constexpr uint64 FILE_B = 0x0202020202020202ULL;
constexpr uint64 FILE_C = 0x0404040404040404ULL;
constexpr uint64 FILE_D = 0x0808080808080808ULL;
constexpr uint64 FILE_E = 0x1010101010101010ULL;
constexpr uint64 FILE_F = 0x2020202020202020ULL;
constexpr uint64 FILE_G = 0x4040404040404040ULL;
constexpr uint64 FILE_H = 0x8080808080808080ULL;

constexpr uint64 RANK_1 = 0x00000000000000FFULL;
constexpr uint64 RANK_2 = 0x000000000000FF00ULL;
constexpr uint64 RANK_3 = 0x0000000000FF0000ULL;
constexpr uint64 RANK_4 = 0x00000000FF000000ULL;
constexpr uint64 RANK_5 = 0x000000FF00000000ULL;
constexpr uint64 RANK_6 = 0x0000FF0000000000ULL;
constexpr uint64 RANK_7 = 0x00FF000000000000ULL;
constexpr uint64 RANK_8 = 0xFF00000000000000ULL;

constexpr uint64 FILES[] = {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
constexpr uint64 RANKS[] = {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};

constexpr uint64 LIGHT_SQUARES = 0x55AA55AA55AA55AA;
constexpr uint64 DARK_SQUARES = ~LIGHT_SQUARES;

enum SQ {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	NO_SQUARE
};

enum Color { White = 0, Black = 1 };
enum PieceIndex { WPawn = 0, WKnight = 1, WBishop = 2, WRook = 3, WQueen = 4, WKing = 5, BPawn = 6, BKnight = 7, BBishop = 8, BRook = 9, BQueen = 10, BKing = 11};
enum BitboardIndex { AllIndex = 12, WhiteIndex = 13, BlackIndex = 14 };

enum GameResult { InProgress = 0, WhiteWin = 1, BlackWin = 2, Stalemate = 3, AgreeToDraw = 4, ThreeFoldRepetition = 5, FiftyMoveRule = 6, InsufficientMaterial = 7,
		WhiteTimeOut = 8, BlackTimeOut = 9, WhiteResign = 10, BlackResign = 11 }; 

enum SearchGameResult { NotDone, Draw, Checkmate };

constexpr int8 STANDARD_PIECE_VALUES[12] = {1,3,3,5,9,0,1,3,3,5,9,0};
constexpr uint8 EMPTY = 15;
constexpr uint8 PIECE_COUNT = 12;

constexpr int16 NEG_INF = INT16_MIN + 1;
constexpr int16 POS_INF = INT16_MAX;

constexpr uint16 MAX_MOVE_COUNT = 218;

static constexpr std::string_view DEFAULT_FEN_POSITION("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0");

inline uint16 getPieceType(Piece piece) { return piece % 6; }
inline Color getPieceColor(Piece piece) { return piece < 6 ? White : Black; } 
inline bool isWhite(Piece piece) { return piece < 6; }
inline bool isBlack(Piece piece) { return piece >= 6; }

static inline Piece charToPiece(char c) {
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
	std::cerr << "Invalid char to convert to piece: " << c << std::endl;
	return WPawn;
}

static inline uint16 squareCharToInt(char c) {
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

static inline char pieceToChar(Piece p) {
	switch (p) {
		case WPawn: return 'p' - 32;
		case WKnight: return 'n' - 32;
		case WBishop: return 'b' - 32;
		case WRook: return 'r' - 32;
		case WQueen: return 'q' - 32;
		case WKing: return 'k' - 32;
		case BPawn: return 'p';
		case BKnight: return 'n';
		case BBishop: return 'b';
		case BRook: return 'r';
		case BQueen: return 'q';
		case BKing: return 'k';
		case EMPTY:return '\0';
	}
	return '\0';
}

constexpr uint32 uint32_log2(uint32 v) {
	assert(v != 0);
	return std::bit_width(v) - 1;
}

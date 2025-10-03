#pragma once
#include <cstdint>
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
constexpr uint16 QUEEN_SIDE_FLAG =	  0b0101;

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

enum Color { White = 0, Black = 1 };
enum PieceIndex { WPawn = 0, WKnight = 1, WBishop = 2, WRook = 3, WQueen = 4, WKing = 5, BPawn = 6, BKnight = 7, BBishop = 8, BRook = 9, BQueen = 10, BKing = 11};
enum BitboardIndex { AllIndex = 12, WhiteIndex = 13, BlackIndex = 14 };
enum GameResult { InProgress, WhiteWin, BlackWin, Stalemate, AgreeToDraw, ThreeFoldRepetition, FiftyMoveRule, InsufficientMaterial, WhiteTimeOut, BlackTimeOut, WhiteResign, BlackResign }; 

constexpr uint8 EMPTY = 15;
constexpr uint8 PIECE_COUNT = 12;

constexpr std::string_view DEFAULT_FEN_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0";

inline uint16 getPieceType(Piece piece) { return piece % 6; }
inline bool isWhite(Piece piece) { return piece < 6; }
inline bool isBlack(Piece piece) { return piece >= 6; }



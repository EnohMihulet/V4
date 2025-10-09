#include <iostream>
#include <ostream>
#include <vector>

#include "MoveGen.h"


bool isSquareAttacked(const GameState& gameState, uint64 pos, Color color) {
	Bitboard pawns = (color == White) ? gameState.bitboards[WPawn] : gameState.bitboards[BPawn];

	int16 leftShift = (color == White) ? 7 : -9;
	int16 rightShift = (color == White) ? 9 : -7;

	if (((leftShift > 0 ? (pawns & ~FILE_A) << leftShift : (pawns & ~FILE_A) >> -leftShift) & pos) != 0) return true;
	if (((rightShift > 0 ? (pawns & ~FILE_H) << rightShift : (pawns & ~FILE_H) >> -rightShift) & pos) != 0) return true;

	Bitboard knights = (color == White) ? gameState.bitboards[WKnight] : gameState.bitboards[BKnight];

	if ((((knights & ~(FILE_A | FILE_B | RANK_8)) << 6) & pos) != 0) return true;
	if ((((knights & ~(FILE_G | FILE_H | RANK_8)) << 10) & pos) != 0) return true;
	if ((((knights & ~(FILE_A | RANK_7 | RANK_8)) << 15) & pos) != 0) return true;
	if ((((knights & ~(FILE_H | RANK_7 | RANK_8)) << 17) & pos) != 0) return true;
	if ((((knights & ~(FILE_A | FILE_B | RANK_1)) >> 10) & pos) != 0) return true;
	if ((((knights & ~(FILE_G | FILE_H | RANK_1)) >> 6) & pos) != 0) return true;
	if ((((knights & ~(FILE_A | RANK_1 | RANK_2)) >> 17) & pos) != 0) return true;
	if ((((knights & ~(FILE_H | RANK_1 | RANK_2)) >> 15) & pos) != 0) return true;

	Bitboard bishops = (color == White) ? gameState.bitboards[WBishop] : gameState.bitboards[BBishop];
	Bitboard empty = ~gameState.bitboards[AllIndex];

	constexpr int16 directions[8] = {7, 9, -7, -9, 1, 8, -1, -8};
	constexpr uint64 boundryMasks[8] = {FILE_A, FILE_H, FILE_A, FILE_H, FILE_H, RANK_8, FILE_A, RANK_1};

	while (bishops) {
		Bitboard bishop = bishops & -bishops;

		for (uint16 i = 0; i < 4; i++) {
			int16 direction = directions[i]; 
			uint64 boundryMask = boundryMasks[i]; 
			Bitboard current = bishop;
			while (true) {
				current = direction < 0 ? current >> abs(direction) : current << direction;
				if ((current & (boundryMask)) != 0) break;

				if ((current & empty) != 0) continue;
				else if ((current & pos) != 0) return true;
				else break;
			}
		}
		bishops &= bishops - 1;
	}

	Bitboard rooks = color == White ? gameState.bitboards[WRook] : gameState.bitboards[BRook];

	while (rooks) {
		Bitboard rook = rooks & -rooks;

		for (uint16 i = 4; i < 8; i++) {
			int16 direction = directions[i]; 
			uint64 boundryMask = boundryMasks[i]; 
			Bitboard current = rook;
			while (true) {
				current = direction < 0 ? current >> abs(direction) : current << direction;
				if ((current & (boundryMask)) != 0) break;
				if ((current & empty) != 0) continue;
				else if ((current & pos) != 0) return true;
				else break;
			}
		}
		rooks &= rooks - 1;
	}

	Bitboard queens = color == White ? gameState.bitboards[WQueen] : gameState.bitboards[BQueen];

	while (queens) {
		Bitboard queen = queens & -queens;

		for (uint16 i = 0; i < 8; i++) {
			int16 direction = directions[i]; 
			uint64 boundryMask = boundryMasks[i]; 
			Bitboard current = queen;
			while (true) {
				current = direction < 0 ? current >> abs(direction) : current << direction;
				if ((current & (boundryMask)) != 0) break;

				if ((current & empty) != 0) continue;
				else if ((current & pos) != 0) return true; 
				else break;
			}
		}
		queens &= queens - 1;
	}

	return false;
}

void filterMoves(GameState& gameState, std::vector<MoveInfo>& history, std::vector<Move>& moves, Color color) {
	std::vector<Move> filteredMoves;
	filteredMoves.reserve(moves.size());

	Color enemy = (color == White) ? Black : White;
	int kingIndex = (color == White) ? WKing : BKing;

	for (const Move& move : moves) {
		gameState.makeMove(move, history);

		bool legal = false;

		Bitboard kingPos = gameState.bitboards[kingIndex];
		bool kingInCheck = isSquareAttacked(gameState, kingPos, enemy);

		if (move.isKingSideCastle()) {
			constexpr int squaresToCheck[2][2] = {{5, 6}, {61, 62}};
			const int* path = (color == White) ? squaresToCheck[0] : squaresToCheck[1];

			if (!isSquareAttacked(gameState, 1ULL << path[0], enemy) &&
				!isSquareAttacked(gameState, 1ULL << path[1], enemy) &&
				!kingInCheck)
				legal = true;
		}
		else if (move.isQueenSideCastle()) {
			int squaresToCheck[2][3] = {{3, 2, 1}, {59, 58, 57}};
			const int* path = (color == White) ? squaresToCheck[0] : squaresToCheck[1];

			if (!isSquareAttacked(gameState, 1ULL << path[0], enemy) &&
				!isSquareAttacked(gameState, 1ULL << path[1], enemy) &&
				!isSquareAttacked(gameState, 1ULL << path[2], enemy) &&
				!kingInCheck)
				legal = true;
		}
		else if (!kingInCheck) legal = true;

		if (legal) filteredMoves.push_back(move);

		gameState.unmakeMove(move, history);
	}

	moves = std::move(filteredMoves);
}

void generatePawnMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking) {
	Bitboard empty = ~gameState.bitboards[AllIndex];
	Bitboard pawns = (color == White) ? gameState.bitboards[WPawn] : gameState.bitboards[BPawn];
	Bitboard enemys = (color == White) ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];
	uint8 enpassantFile = gameState.enPassantFile;
	Bitboard epFileMask = 0ULL;
	if (enpassantFile != NO_ENPASSANT_FILE) epFileMask = FILE_A << enpassantFile;

	int16 pushDir = (color == White) ? 8 : -8;
	Bitboard doublePushRank = (color == White) ? RANK_3 : RANK_6;
	uint16 promotionRank = (color == White) ? 56 : 7;

	int16 leftShift = (color == White) ? 7 : -9;
	int16 rightShift = (color == White) ? 9 : -7;

	auto pushLoop = [&](Bitboard bb, int16 shift, uint16 promotionRank) {
		while (bb) {
			uint16 to = __builtin_ctzll(bb);
			uint16 from = to - shift;
			if ((color == White && to >= promotionRank) || (color == Black && to <= promotionRank)) {
				moves.push_back(Move(from, to, QUEEN_PROMOTE_FLAG));
				moves.push_back(Move(from, to, KNIGHT_PROMOTE_FLAG));
				moves.push_back(Move(from, to, ROOK_PROMOTE_FLAG));
				moves.push_back(Move(from, to, BISHOP_PROMOTE_FLAG));
			} else moves.push_back(Move(from, to, NO_FLAG));
			bb &= bb - 1;
		}
	};

	auto doublePushLoop = [&](Bitboard bb, int16 shift) {
		while (bb) {
			uint16 to = __builtin_ctzll(bb);
			uint16 from = to - shift;
			moves.push_back(Move(from, to, PAWN_TWO_UP_FLAG));
			bb &= bb - 1;
		}
	};

	auto captureLoop = [&](Bitboard bb, int16 shift, uint16 promotionRank) {
		while (bb) {
			uint16 to = __builtin_ctzll(bb);
			uint16 from = to - shift;
			if ((color == White && to >= promotionRank) || (color == Black && to <= promotionRank)) {
				moves.push_back(Move(from, to, QUEEN_PROMOTE_CAPTURE));
				moves.push_back(Move(from, to, KNIGHT_PROMOTE_CAPTURE));
				moves.push_back(Move(from, to, ROOK_PROMOTE_CAPTURE));
				moves.push_back(Move(from, to, BISHOP_PROMOTE_CAPTURE));
			} else moves.push_back(Move(from, to, CAPTURE_FLAG));
			bb &= bb - 1;
		}
	};

	auto epLoop = [&](Bitboard bb, int16 shift) {
		while (bb) {
			uint16 to = __builtin_ctzll(bb);
			uint16 from = to - shift;
			moves.push_back(Move(from, to, EN_PASSANT_FLAG));
			bb &= bb - 1;
		}
	};

	if (!onlyAttacking) {
		Bitboard singlePushes = (pushDir > 0 ? pawns << pushDir : pawns >> -pushDir) & empty;
		Bitboard doublePushes = ((singlePushes & doublePushRank) << (pushDir > 0 ? pushDir : 0)) & empty;
		if (pushDir < 0) doublePushes = ((singlePushes & doublePushRank) >> -pushDir) & empty;
		pushLoop(singlePushes, pushDir, promotionRank);
		doublePushLoop(doublePushes, 2*pushDir);
	}

	Bitboard leftCaptures = (leftShift > 0 ? (pawns & ~FILE_A) << leftShift : (pawns & ~FILE_A) >> -leftShift) & enemys;
	Bitboard rightCaptures = (rightShift > 0 ? (pawns & ~FILE_H) << rightShift : (pawns & ~FILE_H) >> -rightShift) & enemys;

	Bitboard epSquare = (epFileMask & ((color == White) ? RANK_6 : RANK_3));
	Bitboard leftEnpassant;
	Bitboard rightEnpassant;
	if (leftShift > 0) leftEnpassant = (pawns & ~FILE_A) << leftShift & epSquare;
	else leftEnpassant = (pawns & ~FILE_A) >> -leftShift & epSquare;

	if (rightShift > 0) rightEnpassant = (pawns & ~FILE_H) << rightShift & epSquare;
	else rightEnpassant = (pawns & ~FILE_H) >> -rightShift & epSquare;

	captureLoop(leftCaptures, leftShift, promotionRank);
	captureLoop(rightCaptures, rightShift, promotionRank);
	epLoop(leftEnpassant, leftShift);
	epLoop(rightEnpassant, rightShift);
}

void generateKnightMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking) {
	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard knights = color == White ? gameState.bitboards[WKnight] : gameState.bitboards[BKnight];
	Bitboard enemies = color == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	Bitboard leftUp = ((knights & ~(FILE_A | FILE_B | RANK_8)) << 6);
	Bitboard rightUp = ((knights & ~(FILE_G | FILE_H | RANK_8)) << 10);
	Bitboard upLeft = ((knights & ~(FILE_A | RANK_7 | RANK_8)) << 15);
	Bitboard upRight = ((knights & ~(FILE_H | RANK_7 | RANK_8)) << 17);
	Bitboard leftDown = ((knights & ~(FILE_A | FILE_B | RANK_1)) >> 10);
	Bitboard rightDown = ((knights & ~(FILE_G | FILE_H | RANK_1)) >> 6);
	Bitboard downLeft = ((knights & ~(FILE_A | RANK_1 | RANK_2)) >> 17);
	Bitboard downRight = ((knights & ~(FILE_H | RANK_1 | RANK_2)) >> 15);

	auto captureLoop = [&](Bitboard bb, int16 shift) {
		while (bb) {
			uint16 to = __builtin_ctzll(bb);
			uint16 from = to - shift;
			moves.push_back(Move(from, to, CAPTURE_FLAG));
			bb &= bb - 1;
		}
	};

	Bitboard leftUpCapture = leftUp & enemies;
	Bitboard rightUpCapture = rightUp & enemies;
	Bitboard upLeftCapture = upLeft & enemies;
	Bitboard upRightCapture = upRight & enemies;
	Bitboard leftDownCapture = leftDown & enemies;
	Bitboard rightDownCapture = rightDown & enemies;
	Bitboard downLeftCapture = downLeft & enemies;
	Bitboard downRightCapture = downRight & enemies;

	captureLoop(leftUpCapture, 6);
	captureLoop(rightUpCapture, 10);
	captureLoop(upLeftCapture, 15);
	captureLoop(upRightCapture, 17);
	captureLoop(leftDownCapture, -10);
	captureLoop(rightDownCapture, -6);
	captureLoop(downLeftCapture, -17);
	captureLoop(downRightCapture, -15);

	if (!onlyAttacking) {
		auto moveLoop = [&](Bitboard bb, int16 shift) {
			while (bb) {
				uint16 to = __builtin_ctzll(bb);
				uint16 from = to - shift;
				moves.push_back(Move(from, to, NO_FLAG));
				bb &= bb - 1;
			}
		};

		Bitboard leftUpNoFlag = leftUp & empty;
		Bitboard rightUpNoFlag = rightUp & empty;
		Bitboard upLeftNoFlag = upLeft & empty;
		Bitboard upRightNoFlag = upRight & empty;
		Bitboard leftDownNoFlag = leftDown & empty;
		Bitboard rightDownNoFlag = rightDown & empty;
		Bitboard downLeftNoFlag = downLeft & empty;
		Bitboard downRightNoFlag = downRight & empty;

		moveLoop(leftUpNoFlag, 6);
		moveLoop(rightUpNoFlag, 10);
		moveLoop(upLeftNoFlag, 15);
		moveLoop(upRightNoFlag, 17);
		moveLoop(leftDownNoFlag, -10);
		moveLoop(rightDownNoFlag, -6);
		moveLoop(downLeftNoFlag, -17);
		moveLoop(downRightNoFlag, -15);
	}
}

void generateBishopMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking) {
	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard bishops = color == White ? gameState.bitboards[WBishop] : gameState.bitboards[BBishop];
	Bitboard enemies = color == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	constexpr int16 directions[4] = {7, 9, -7, -9};
	constexpr uint64 boundryMasks[4] = {FILE_H, FILE_A, FILE_A, FILE_H};

	while (bishops) {
		Bitboard bishop = bishops & -bishops;
		uint16 from = __builtin_ctzll(bishop);

		for (uint16 i = 0; i < 4; i++) {
			int16 direction = directions[i]; 
			uint64 boundryMask = boundryMasks[i]; 
			Bitboard current = bishop;
			while (true) {
				current = direction < 0 ? current >> abs(direction) : current << direction;
				if ((current & boundryMask) != 0) break;

				if ((current & empty) != 0) {
					if (!onlyAttacking) moves.push_back(Move(from, __builtin_ctzll(current), NO_FLAG));
				}
				else if ((current & enemies) != 0) {
					moves.push_back(Move(from, __builtin_ctzll(current), CAPTURE_FLAG));
					break;
				}
				else break;
			}
		}
		bishops &= bishops - 1;
	}
}

void generateRookMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking) {
	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard rooks = color == White ? gameState.bitboards[WRook] : gameState.bitboards[BRook];
	Bitboard enemies = color == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	constexpr int16 directions[4] = {1, 8, -1, -8};
	constexpr uint64 boundryMasks[4] = {FILE_A, RANK_1, FILE_H, RANK_8};

	while (rooks) {
		Bitboard rook = rooks & -rooks;
		uint16 from = __builtin_ctzll(rook);

		for (uint16 i = 0; i < 4; i++) {
			int16 direction = directions[i]; 
			uint64 boundryMask = boundryMasks[i]; 
			Bitboard current = rook;
			while (true) {
				current = direction < 0 ? current >> abs(direction) : current << direction;
				if ((current & boundryMask) != 0) break;

				if ((current & empty) != 0) {
					if (!onlyAttacking) moves.push_back(Move(from, __builtin_ctzll(current), NO_FLAG));
				}
				else if ((current & enemies) != 0) {
					moves.push_back(Move(from, __builtin_ctzll(current), CAPTURE_FLAG));
					break;
				}
				else break;
			}
		}
		rooks &= rooks - 1;
	}
}

void generateQueenMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking) {
	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard queens = color == White ? gameState.bitboards[WQueen] : gameState.bitboards[BQueen];
	Bitboard enemies = color == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	constexpr int16 directions[8] = {1, 8, -1, -8, 7, 9, -7, -9};
	constexpr uint64 boundryMasks[8] = {FILE_A, RANK_1, FILE_H, RANK_8, FILE_H, FILE_A, FILE_A, FILE_H};

	while (queens) {
		Bitboard queen = queens & -queens;
		uint16 from = __builtin_ctzll(queen);

		for (uint16 i = 0; i < 8; i++) {
			int16 direction = directions[i]; 
			uint64 boundryMask = boundryMasks[i]; 
			Bitboard current = queen;
			while (true) {
				current = direction < 0 ? current >> abs(direction) : current << direction;
				if ((current & boundryMask) != 0) break;

				if ((current & empty) != 0) {
					if (!onlyAttacking) moves.push_back(Move(from, __builtin_ctzll(current), NO_FLAG));
				}
				else if ((current & enemies) != 0) {
					moves.push_back(Move(from, __builtin_ctzll(current), CAPTURE_FLAG));
					break;
				}
				else break;
			}
		}
		queens &= queens - 1;
	}
}

void generateKingMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking) {
	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard king = color == White ? gameState.bitboards[WKing] : gameState.bitboards[BKing];
	Bitboard enemies = color == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	constexpr int16 directions[8] = {1, 8, -1, -8, 7, 9, -7, -9};
	constexpr uint64 boundryMasks[8] = {FILE_A, RANK_1, FILE_H, RANK_8, FILE_H, FILE_A, FILE_A, FILE_H};

	uint16 from = __builtin_ctzll(king);

	for (uint16 i = 0; i < 8; i++) {
		int16 direction = directions[i]; 
		uint64 boundryMask = boundryMasks[i]; 
		Bitboard current = direction < 0 ? king >> abs(direction) : king << direction;;

		if ((current & (boundryMask)) != 0) continue;

		if ((current & empty) != 0) {
			if (!onlyAttacking) moves.push_back(Move(from, __builtin_ctzll(current), NO_FLAG));
		}
		else if ((current & enemies) != 0) {
			moves.push_back(Move(from, __builtin_ctzll(current), CAPTURE_FLAG));
		}
	}

	if (color == White) {
		if ((gameState.castlingRights & W_KING_SIDE) &&
			(empty & ((1ULL << 5) | (1ULL << 6))) == ((1ULL << 5) | (1ULL << 6))) {
			moves.push_back(Move(from, 6, KING_SIDE_FLAG));
		}
		if ((gameState.castlingRights & W_QUEEN_SIDE) &&
			(empty & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) == ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) {
			moves.push_back(Move(from, 2, QUEEN_SIDE_FLAG));
		}
	}
	else {
		if ((gameState.castlingRights & B_KING_SIDE) &&
			(empty & ((1ULL << 61) | (1ULL << 62))) == ((1ULL << 61) | (1ULL << 62))) {
			moves.push_back(Move(from, 62, KING_SIDE_FLAG));
		}
		if ((gameState.castlingRights & B_QUEEN_SIDE) &&
			(empty & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) == ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) {
			moves.push_back(Move(from, 58, QUEEN_SIDE_FLAG));
		}
	}
}

void generateAllMoves(const GameState& gameState, std::vector<Move>& moves, Color color, bool onlyAttacking) {
	generatePawnMoves(gameState, moves, color, onlyAttacking);
	generateKnightMoves(gameState, moves, color, onlyAttacking);
	generateBishopMoves(gameState, moves, color, onlyAttacking);
	generateRookMoves(gameState, moves, color, onlyAttacking);
	generateQueenMoves(gameState, moves, color, onlyAttacking);
	generateKingMoves(gameState, moves, color, onlyAttacking);
}

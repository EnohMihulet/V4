#include <iostream>
#include <vector>

#include "MoveGen.h"

#include "PrecomputedTables.h"
#include "../helpers/GameStateHelper.h"

std::vector<MoveInfo> g_EPHistory;

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

Bitboard getPossibleBishopAttackers(uint8 square, Bitboard occupied) {
	Bitboard allAttackers = 0ULL;

	Bitboard ray = RAY_MASK[UP_LEFT_RAY_TABLE_INDEX][square];
	Bitboard blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = __builtin_ctzll(blockers);
		allAttackers |= RAY_BETWEEN[square][blockerSquare] | (1ULL << blockerSquare);
	}

	ray = RAY_MASK[UP_RIGHT_RAY_TABLE_INDEX][square];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = __builtin_ctzll(blockers);
		allAttackers |= RAY_BETWEEN[square][blockerSquare] | (1ULL << blockerSquare);
	}

	ray = RAY_MASK[DOWN_LEFT_RAY_TABLE_INDEX][square];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = 63 - __builtin_clzll(blockers);
		allAttackers |= RAY_BETWEEN[square][blockerSquare] | (1ULL << blockerSquare);
	}

	ray = RAY_MASK[DOWN_RIGHT_RAY_TABLE_INDEX][square];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = 63 - __builtin_clzll(blockers);
		allAttackers |= RAY_BETWEEN[square][blockerSquare] | (1ULL << blockerSquare);
	}
	return allAttackers;
}

Bitboard getPossibleRookAttackers(uint8 square, Bitboard occupied) {
	Bitboard allAttackers = 0ULL;

	Bitboard ray = RAY_MASK[RIGHT_RAY_TABLE_INDEX][square];
	Bitboard blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = __builtin_ctzll(blockers);
		allAttackers |= RAY_BETWEEN[square][blockerSquare] | (1ULL << blockerSquare);
	}

	ray = RAY_MASK[LEFT_RAY_TABLE_INDEX][square];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = 63 - __builtin_clzll(blockers);
		allAttackers |= RAY_BETWEEN[square][blockerSquare] | (1ULL << blockerSquare);
	}

	ray = RAY_MASK[UP_RAY_TABLE_INDEX][square];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = __builtin_ctzll(blockers);
		allAttackers |= RAY_BETWEEN[square][blockerSquare] | (1ULL << blockerSquare);
	}

	ray = RAY_MASK[DOWN_RAY_TABLE_INDEX][square];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = 63 - __builtin_clzll(blockers);
		allAttackers |= RAY_BETWEEN[square][blockerSquare] | (1ULL << blockerSquare);
	}

	return allAttackers;
}

void computeCheckAndPinMasks(const GameState& gameState, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays) {
	Bitboard king = us == White ? gameState.bitboards[WKing] : gameState.bitboards[BKing];
	uint8 kingSq; 
	if (king) kingSq = __builtin_ctzll(king);
	else return;

	Bitboard enemyPawn, enemyBishop, enemyKnight, enemyRook, enemyQueen;
	Color them;
	uint8 ourIndex;
	if (us == White) {
		enemyPawn = gameState.bitboards[BPawn];
		enemyBishop = gameState.bitboards[BBishop];
		enemyKnight = gameState.bitboards[BKnight];
		enemyRook = gameState.bitboards[BRook];
		enemyQueen = gameState.bitboards[BQueen];
		them = Black;
		ourIndex = WhiteIndex;
	}
	else {
		enemyPawn = gameState.bitboards[WPawn];
		enemyBishop = gameState.bitboards[WBishop];
		enemyKnight = gameState.bitboards[WKnight];
		enemyRook = gameState.bitboards[WRook];
		enemyQueen = gameState.bitboards[WQueen];
		them = White;
		ourIndex = BlackIndex;
	}

	Bitboard occupied = gameState.bitboards[AllIndex];

	Bitboard pawnAttackers = PAWN_ATTACK_TABLE[them][kingSq] & enemyPawn;
	Bitboard bishopAttackers = getPossibleBishopAttackers(kingSq, occupied) & (enemyBishop | enemyQueen);
	Bitboard knightAttackers = KNIGHT_ATTACK_TABLE[kingSq] & enemyKnight;
	Bitboard rookAttackers = getPossibleRookAttackers(kingSq, occupied) & (enemyRook | enemyQueen);
	Bitboard checkers = pawnAttackers | bishopAttackers | knightAttackers | rookAttackers;
	uint8 checkersCount = __builtin_popcountll(checkers);

	if (checkersCount == 0) checkMask = ~0ULL;
	else if (checkersCount == 1) {
		uint8 checkerSq = __builtin_ctzll(checkers);
		checkMask = RAY_BETWEEN[kingSq][checkerSq] | (1ULL << checkerSq);
	}

	for (int dirIdx = 0; dirIdx < 8; ++dirIdx) {
		const Bitboard ray = RAY_MASK[kingSq][dirIdx];
		Bitboard blockers = ray & occupied;
		if (!blockers) continue;

		const bool dec = DIRECTION_DECREASES[dirIdx];
		const uint8 firstSq = dec ? (63 - __builtin_clzll(blockers)) : __builtin_ctzll(blockers);

		blockers &= blockers - 1;
		if (!blockers) continue;

		if (gameState.bitboards[ourIndex] & (1ULL << firstSq)) {
			const uint8 secondSq = dec ? (63 - __builtin_clzll(blockers)) : __builtin_ctzll(blockers);
			const Piece secondPiece = gameState.pieceAt(secondSq);

			const bool isStraightRay = (dirIdx <= 3);
			const Piece enemyRook   = (us == White ? BRook : WRook);
			const Piece enemyBishop = (us == White ? BBishop : WBishop);
			const Piece enemyQueen  = (us == White ? BQueen : WQueen);

			if ((isStraightRay && secondPiece == enemyRook) || (!isStraightRay && secondPiece == enemyBishop) || (secondPiece == enemyQueen)) {
				pinnedPieces |= (1ULL << firstSq);
				pinnedRays[firstSq] = RAY_MASK[kingSq][secondSq] | (1ULL << secondSq);
			}
		}
	}
}

void generatePawnMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays) {

	auto pushLoop = [&](Bitboard bb, int16 shift, uint16 promotionRank) {
		while (bb) {
			uint16 to = __builtin_ctzll(bb);
			uint16 from = to - shift;

			if ((pinnedPieces & (1ULL << from)) && !(pinnedRays[from] & (1ULL << to))) {
				bb &= bb - 1;
				continue;
			}

			if ((us == White && to/8 >= promotionRank) || (us == Black && to/8 <= promotionRank)) {
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

			if (pinnedPieces & (1ULL << from) && !(pinnedRays[from] & (1ULL << to))) {
				bb &= bb - 1;
				continue;
			}

			moves.push_back(Move(from, to, PAWN_TWO_UP_FLAG));
			bb &= bb - 1;
		}
	};

	auto captureLoop = [&](Bitboard bb, int16 shift, uint16 promotionRank) {
		while (bb) {
			uint16 to = __builtin_ctzll(bb);
			uint16 from = to - shift;

			if (pinnedPieces & (1ULL << from) && !(pinnedRays[from] & (1ULL << to))) {
				bb &= bb - 1;
				continue;
			}

			if ((us == White && to/8 >= promotionRank) || (us == Black && to/8 <= promotionRank)) {
				moves.push_back(Move(from, to, QUEEN_PROMOTE_CAPTURE));
				moves.push_back(Move(from, to, KNIGHT_PROMOTE_CAPTURE));
				moves.push_back(Move(from, to, ROOK_PROMOTE_CAPTURE));
				moves.push_back(Move(from, to, BISHOP_PROMOTE_CAPTURE));
			} else moves.push_back(Move(from, to, CAPTURE_FLAG));
			bb &= bb - 1;
		}
	};

	Bitboard empty = ~gameState.bitboards[AllIndex];

	uint8 enpassantFile = gameState.enPassantFile;
	Bitboard epFileMask = 0ULL;
	if (enpassantFile != NO_ENPASSANT_FILE) epFileMask = FILE_A << enpassantFile;

	if (us == White) {
		Bitboard pawns = gameState.bitboards[WPawn];
		Bitboard enemies = gameState.bitboards[BlackIndex];
		Bitboard enemyPawns = gameState.bitboards[BPawn];
		
		Bitboard singlePushes = pawns << 8 & empty & checkMask;
		Bitboard doublePushes = ((pawns << 8) & empty & RANK_3) << 8 & empty & checkMask; // Recomputing single push handles case where double push blocks check

		pushLoop(singlePushes, 8, 7);
		doublePushLoop(doublePushes, 16);

		Bitboard leftCaptures = (pawns & ~FILE_A) << 7 & enemies & checkMask;
		Bitboard rightCaptures = (pawns & ~FILE_H) << 9 & enemies & checkMask;

		captureLoop(leftCaptures, 7, 7);
		captureLoop(rightCaptures, 9, 7);

		Bitboard epSquare = epFileMask & RANK_6;

		Bitboard leftEnpassant = (pawns & ~FILE_A) << 7 & epSquare & checkMask;
		Bitboard rightEnpassant = (pawns & ~FILE_H) << 9 & epSquare & checkMask;

		if (leftEnpassant) {
			uint16 to = __builtin_ctzll(leftEnpassant);
			uint16 from = to - 7;
			Move epMove{from, to, EN_PASSANT_FLAG};

			gameState.makeMove(epMove, g_EPHistory);
			if (!isSquareAttacked(gameState, gameState.bitboards[WKing], Black)) moves.push_back(epMove);
			gameState.unmakeMove(epMove, g_EPHistory);
		}
		if (rightEnpassant) {
			uint16 to = __builtin_ctzll(rightEnpassant);
			uint16 from = to - 9;
			Move epMove{from, to, EN_PASSANT_FLAG};

			gameState.makeMove(epMove, g_EPHistory);
			if (!isSquareAttacked(gameState, gameState.bitboards[WKing], Black)) moves.push_back(epMove);
			gameState.unmakeMove(epMove, g_EPHistory);
		}
	}
	else {
		Bitboard pawns = gameState.bitboards[BPawn];
		Bitboard enemies = gameState.bitboards[WhiteIndex];
		Bitboard enemyPawns = gameState.bitboards[WPawn];
		
		Bitboard singlePushes = pawns >> 8 & empty & checkMask;
		Bitboard doublePushes = ((pawns >> 8) & empty & RANK_6) >> 8 & empty & checkMask;

		pushLoop(singlePushes, -8, 0);
		doublePushLoop(doublePushes, -16);

		Bitboard leftCaptures = (pawns & ~FILE_A) >> 9 & enemies & checkMask;
		Bitboard rightCaptures = (pawns & ~FILE_H) >> 7 & enemies & checkMask;

		captureLoop(leftCaptures, -9, 0);
		captureLoop(rightCaptures, -7, 0);

		Bitboard epSquareMask = epFileMask & RANK_3;

		Bitboard leftEnpassant = (pawns & ~FILE_A) >> 9 & epSquareMask & checkMask;
		Bitboard rightEnpassant = (pawns & ~FILE_H) >> 7 & epSquareMask & checkMask;

		if (leftEnpassant) {
			uint16 to = __builtin_ctzll(leftEnpassant);
			uint16 from = to + 9;
			Move epMove{from, to, EN_PASSANT_FLAG};

			gameState.makeMove(epMove, g_EPHistory);
			if (!isSquareAttacked(gameState, gameState.bitboards[BKing], White)) moves.push_back(epMove);
			gameState.unmakeMove(epMove, g_EPHistory);
		}
		if (rightEnpassant) {
			uint16 to = __builtin_ctzll(rightEnpassant);
			uint16 from = to + 7;
			Move epMove{from, to, EN_PASSANT_FLAG};

			gameState.makeMove(epMove, g_EPHistory);
			if (!isSquareAttacked(gameState, gameState.bitboards[BKing], White)) moves.push_back(epMove);
			gameState.unmakeMove(epMove, g_EPHistory);
		}
	}
}

void generateKnightMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays) {
	auto moveLoop = [&](Bitboard bb, uint8 from) {
		while (bb) {
			uint8 to = __builtin_ctzll(bb);

			moves.push_back(Move(from, to, NO_FLAG));

			bb &= bb - 1;
		}
	};

	auto captureLoop = [&](Bitboard bb, uint8 from) {
		while (bb) {
			uint8 to = __builtin_ctzll(bb);

			moves.push_back(Move(from, to, CAPTURE_FLAG));

			bb &= bb - 1;
		}
	};

	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard knights = us == White ? gameState.bitboards[WKnight] : gameState.bitboards[BKnight];
	Bitboard enemies = us == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	while (knights) {
		uint8 from = __builtin_ctzll(knights);
		Bitboard noncaptures = KNIGHT_ATTACK_TABLE[from] & empty & checkMask & ~(pinnedPieces); // Pinned knight can never move
		Bitboard captures = KNIGHT_ATTACK_TABLE[from] & enemies & checkMask & ~(pinnedPieces);
		
		moveLoop(noncaptures, from);
		captureLoop(captures, from);

		knights &= knights - 1;
	}
}

void generateBishopMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays) {
	auto moveLoop = [&](Bitboard bb, uint8 from) {
		while (bb) {
			uint8 to = __builtin_ctzll(bb);

			if (!(pinnedPieces & (1ULL << from) && !(pinnedRays[from] & (1ULL << to)))) {
				moves.push_back(Move(from, to, NO_FLAG));
			}

			bb &= bb - 1;
		}
	};

	auto captureLoop = [&](Bitboard bb, uint8 from) {
		if (bb) {
			uint8 to = __builtin_ctzll(bb);

			if (!(pinnedPieces & (1ULL << from) && !(pinnedRays[from] & (1ULL << to)))) {
				moves.push_back(Move(from, to, CAPTURE_FLAG));
			}

			bb &= bb - 1;
		}
	};

	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard all = gameState.bitboards[AllIndex];
	Bitboard bishops = us == White ? gameState.bitboards[WBishop] : gameState.bitboards[BBishop];
	Bitboard enemies = us == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	while (bishops) {
		uint8 bishopSq = __builtin_ctzll(bishops);

		for (int8 i = 0; i < 4; i++) {
			uint8 directionIndex = DIAGONAL_RAY_TABLE_INDICIES[i];
			Bitboard diagonalRay = RAY_MASK[bishopSq][directionIndex];
			Bitboard diagonalBitboard = diagonalRay & checkMask;

			if (diagonalBitboard) {
				uint8 firstPieceSq;
				bool isDecreasing = DIAGONAL_DECREASES[i]; 

				Bitboard intersectionWithPiece = all & diagonalBitboard;
				if (intersectionWithPiece) {
					if (isDecreasing) firstPieceSq = 63 - __builtin_clzll(intersectionWithPiece);
					else firstPieceSq = __builtin_ctzll(intersectionWithPiece);
					Bitboard nonCaptures = RAY_BETWEEN[bishopSq][firstPieceSq];
					moveLoop(nonCaptures, bishopSq);

					if ((1ULL << firstPieceSq) & enemies) captureLoop(intersectionWithPiece, bishopSq);
					
				}
				else moveLoop(diagonalBitboard, bishopSq);
			}
		}
		bishops &= bishops - 1;
	}
}

void generateRookMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays) {
	auto moveLoop = [&](Bitboard bb, uint8 from) {
		while (bb) {
			uint8 to = __builtin_ctzll(bb);

			if (!(pinnedPieces & (1ULL << from) && !(pinnedRays[from] & (1ULL << to)))) {
				moves.push_back(Move(from, to, NO_FLAG));
			}

			bb &= bb - 1;
		}
	};

	auto captureLoop = [&](Bitboard bb, uint8 from) {
		if (bb) {
			uint8 to = __builtin_ctzll(bb);

			if (!(pinnedPieces & (1ULL << from) && !(pinnedRays[from] & (1ULL << to)))) {
				moves.push_back(Move(from, to, CAPTURE_FLAG));
			}

			bb &= bb - 1;
		}
	};

	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard all = gameState.bitboards[AllIndex];
	Bitboard rooks = us == White ? gameState.bitboards[WRook] : gameState.bitboards[BRook];
	Bitboard enemies = us == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	while (rooks) {
		uint8 rookSq = __builtin_ctzll(rooks);

		for (int8 i = 0; i < 4; i++) {
	 		uint8 directionIndex = STRAIGHT_RAY_TABLE_INDICIES[i];
			Bitboard straightRay = RAY_MASK[rookSq][directionIndex];
			Bitboard straightBitboard = straightRay & checkMask;

			if (straightBitboard) {
				Bitboard intersectionWithPiece = all & straightBitboard;

				if (intersectionWithPiece) {
					bool isDecreasing = STRAIGHT_DECREASES[i]; 
					uint8 firstPieceSq;

					if (isDecreasing) {
						firstPieceSq = 63 - __builtin_clzll(intersectionWithPiece);
					}
					else  {
						firstPieceSq = __builtin_ctzll(intersectionWithPiece);
					}

					Bitboard nonCaptures = RAY_BETWEEN[rookSq][firstPieceSq];
					moveLoop(nonCaptures, rookSq);

					if ((1ULL << firstPieceSq) & enemies) captureLoop(intersectionWithPiece, rookSq);
				}
				else moveLoop(straightBitboard, rookSq);
			}
		}
		rooks &= rooks - 1;
	}
}

void generateQueenMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays) {
	auto moveLoop = [&](Bitboard bb, uint8 from) {
		while (bb) {
			uint8 to = __builtin_ctzll(bb);

			if (!(pinnedPieces & (1ULL << from) && !(pinnedRays[from] & (1ULL << to)))) {
				moves.push_back(Move(from, to, NO_FLAG));
			}

			bb &= bb - 1;
		}
	};

	auto captureLoop = [&](Bitboard bb, uint8 from) {
		if (bb) {
			uint8 to = __builtin_ctzll(bb);

			if (!(pinnedPieces & (1ULL << from) && !(pinnedRays[from] & (1ULL << to)))) {
				moves.push_back(Move(from, to, NO_FLAG));
			}

			bb &= bb - 1;
		}
	};

	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard all = gameState.bitboards[AllIndex];
	Bitboard queens = us == White ? gameState.bitboards[WQueen] : gameState.bitboards[BQueen];
	Bitboard enemies = us == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	while (queens) {
		uint8 queenSq = __builtin_ctzll(queens);

		for (int8 i = 0; i < 8; i++) {
			int8 directionIndex = RAY_TABLE_INDICIES[i];
			Bitboard ray = RAY_MASK[queenSq][directionIndex];
			Bitboard rayBitboard = ray & checkMask;

			if (rayBitboard) {
				uint8 firstPieceSq;
				bool isDecreasing = DIRECTION_DECREASES[i]; 

				Bitboard intersectionWithPiece = all & rayBitboard;
				if (intersectionWithPiece) {
					if (isDecreasing) firstPieceSq = 63 - __builtin_clzll(intersectionWithPiece);
					else firstPieceSq = __builtin_ctzll(intersectionWithPiece);
					Bitboard nonCaptures = RAY_BETWEEN[queenSq][firstPieceSq];
					moveLoop(nonCaptures, queenSq);

					if ((1ULL << firstPieceSq) & enemies) captureLoop(intersectionWithPiece, queenSq);	
				}
				else moveLoop(rayBitboard, queenSq);
			}
		}
		queens &= queens - 1;
	}
}

void generateKingMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces, std::array<Bitboard, 64>& pinnedRays) {
	Color them = us == White ? Black : White;
	auto moveLoop = [&](Bitboard bb, uint8 from) {
		while (bb) {
			uint8 to = __builtin_ctzll(bb);

			if (!isSquareAttacked(gameState, 1ULL << to, them)) moves.push_back(Move(from, to, NO_FLAG));

			bb &= bb - 1;
		}
	};

	auto captureLoop = [&](Bitboard bb, uint8 from) {
		while (bb) {
			uint8 to = __builtin_ctzll(bb);

			if (!isSquareAttacked(gameState, 1ULL << to, them)) moves.push_back(Move(from, to, CAPTURE_FLAG));

			bb &= bb - 1;
		}
	};

	Bitboard empty = ~(gameState.bitboards[AllIndex]);
	Bitboard kings = us == White ? gameState.bitboards[WKing] : gameState.bitboards[BKing];
	Bitboard enemies = us == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	uint8 from;
	if (kings) from = __builtin_ctzll(kings);
	else return;

	Bitboard noncaptures = KING_ATTACK_TABLE[from] & empty;
	Bitboard captures = KING_ATTACK_TABLE[from] & enemies;
	
	moveLoop(noncaptures, from);
	captureLoop(captures, from);

	if (us == White) {
		if ((gameState.castlingRights & W_KING_SIDE) &&
			(empty & ((1ULL << 5) | (1ULL << 6))) == ((1ULL << 5) | (1ULL << 6))) {
			if (!isSquareAttacked(gameState, 1ULL << 5, them) &&
				!isSquareAttacked(gameState, 1ULL << 6, them) &&
				checkMask == ~0ULL) 
				moves.push_back(Move(from, 6, KING_SIDE_FLAG));
		}
		if ((gameState.castlingRights & W_QUEEN_SIDE) &&
			(empty & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) == ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) {
			if (!isSquareAttacked(gameState, 1ULL << 1, them) &&
				!isSquareAttacked(gameState, 1ULL << 2, them) &&
				!isSquareAttacked(gameState, 1ULL << 3, them) &&
				checkMask == ~0ULL) 
				moves.push_back(Move(from, 2, QUEEN_SIDE_FLAG));
		}
	}
	else {
		if ((gameState.castlingRights & B_KING_SIDE) &&
			(empty & ((1ULL << 61) | (1ULL << 62))) == ((1ULL << 61) | (1ULL << 62))) {
			if (!isSquareAttacked(gameState, 1ULL << 61, them) &&
				!isSquareAttacked(gameState, 1ULL << 62, them) &&
				checkMask == ~0ULL) 
				moves.push_back(Move(from, 62, KING_SIDE_FLAG));
		}
		if ((gameState.castlingRights & B_QUEEN_SIDE) &&
			(empty & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) == ((1ULL << 57) | (1ULL << 58) | (1ULL << 59))) {
			if (!isSquareAttacked(gameState, 1ULL << 57, them) &&
				!isSquareAttacked(gameState, 1ULL << 58, them) &&
				!isSquareAttacked(gameState, 1ULL << 59, them) &&
				checkMask == ~0ULL) 
				moves.push_back(Move(from, 58, QUEEN_SIDE_FLAG));
		}
	}
}

void generateAllMoves(GameState& gameState, std::vector<Move>& moves, Color us) {
	Bitboard pinnedPieces = 0;
	Bitboard checkMask = 0;
	std::array<Bitboard, 64> pinnedRays;
	computeCheckAndPinMasks(gameState, us, checkMask, pinnedPieces, pinnedRays);

	if (checkMask == 0ULL) {
		generateKingMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
		return;
	}

	generatePawnMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateKnightMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateBishopMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateRookMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateQueenMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateKingMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
}

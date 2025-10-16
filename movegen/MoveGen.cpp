#include <iostream>
#include <vector>

#include "MoveGen.h"

#include "PrecomputedTables.h"
#include "../helpers/GameStateHelper.h"

std::vector<MoveInfo> g_EPHistory;

bool isSquareAttacked(const GameState& gameState, uint64 pos, Color them) {
	if (!pos) return false;
	uint8 sq = __builtin_ctzll(pos);
	Color us = (them == White) ? Black : White;

	Bitboard pawns = (them == White) ? gameState.bitboards[WPawn] : gameState.bitboards[BPawn];
	if (PAWN_ATTACK_TABLE[us][sq] & pawns) return true; // Should be us because the sq is the attacked sq is not the pawns square so direction has to be inverted

	Bitboard knights = (them == White) ? gameState.bitboards[WKnight] : gameState.bitboards[BKnight];

	if (KNIGHT_ATTACK_TABLE[sq] & knights) return true;

	const Bitboard kings = (them == White) ? gameState.bitboards[WKing] : gameState.bitboards[BKing];
	if (KING_ATTACK_TABLE[sq] & kings) return true;

	Piece bishopPiece = (them == White) ? WBishop : BBishop;
	Piece rookPiece = (them == White) ? WRook : BRook;
	Piece queenPiece = (them == White) ? WQueen : BQueen;

	Bitboard occupied = gameState.bitboards[AllIndex];

	for (int dirIdx = 0; dirIdx < 8; ++dirIdx) {
		const Bitboard ray = RAY_MASK[sq][dirIdx];
		Bitboard blockers = ray & occupied;
		if (!blockers) continue;

		const bool dec = DIRECTION_DECREASES[dirIdx];
		const uint8 attackerSq = dec ? (63 - __builtin_clzll(blockers)) : __builtin_ctzll(blockers);

		Piece attackerPiece = gameState.pieceAt(attackerSq);
		if (attackerPiece == queenPiece) return true;
		else if (dirIdx <= 3 && attackerPiece == rookPiece) return true;
		else if (attackerPiece == bishopPiece) return true;
	}

	return false;
}

Bitboard getPossibleBishopAttackers(uint8 square, Bitboard occupied) {
	Bitboard allAttackers = 0ULL;

	Bitboard ray = RAY_MASK[square][UP_LEFT_RAY_TABLE_INDEX];
	Bitboard blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = __builtin_ctzll(blockers);
		allAttackers |= 1ULL << blockerSquare;
	}

	ray = RAY_MASK[square][UP_RIGHT_RAY_TABLE_INDEX];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = __builtin_ctzll(blockers);
		allAttackers |= 1ULL << blockerSquare;
	}

	ray = RAY_MASK[square][DOWN_LEFT_RAY_TABLE_INDEX];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = 63 - __builtin_clzll(blockers);
		allAttackers |= 1ULL << blockerSquare;
	}

	ray = RAY_MASK[square][DOWN_RIGHT_RAY_TABLE_INDEX];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = 63 - __builtin_clzll(blockers);
		allAttackers |= 1ULL << blockerSquare;
	}
	return allAttackers;
}

Bitboard getPossibleRookAttackers(uint8 square, Bitboard occupied) {
	Bitboard allAttackers = 0ULL;

	Bitboard ray = RAY_MASK[square][RIGHT_RAY_TABLE_INDEX];
	Bitboard blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = __builtin_ctzll(blockers);
		allAttackers |= 1ULL << blockerSquare;
	}

	ray = RAY_MASK[square][LEFT_RAY_TABLE_INDEX];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = 63 - __builtin_clzll(blockers);
		allAttackers |= 1ULL << blockerSquare;
	}

	ray = RAY_MASK[square][UP_RAY_TABLE_INDEX];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = __builtin_ctzll(blockers);
		allAttackers |= 1ULL << blockerSquare;
	}

	ray = RAY_MASK[square][DOWN_RAY_TABLE_INDEX];
	blockers = ray & occupied;
	if (blockers) {
		uint16 blockerSquare = 63 - __builtin_clzll(blockers);
		allAttackers |= 1ULL << blockerSquare;
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

	Bitboard pawnAttackers = PAWN_ATTACK_TABLE[us][kingSq] & enemyPawn; // Should be us bc direction is inverted because we are starting at the attacked sq
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
	else checkMask = 0ULL;

	for (int dirIdx = 0; dirIdx < 8; ++dirIdx) {
		const Bitboard ray = RAY_MASK[kingSq][dirIdx];
		Bitboard blockers = ray & occupied;
		if (!blockers) continue;

		const bool dec = DIRECTION_DECREASES[dirIdx];
		const uint8 firstSq = dec ? (63 - __builtin_clzll(blockers)) : __builtin_ctzll(blockers);

		if (dec) blockers ^= (1ULL << firstSq);
		else blockers &= (blockers - 1);
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
				pinnedRays[firstSq] = RAY_BETWEEN[kingSq][secondSq] | (1ULL << secondSq);
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
		
		Bitboard singlePushes = pawns << 8 & empty & checkMask;
		Bitboard doublePushes = ((pawns << 8) & empty & RANK_3) << 8 & empty & checkMask; // Recomputing single push handles case where double push blocks check

		pushLoop(singlePushes, 8, 7);
		doublePushLoop(doublePushes, 16);

		Bitboard leftCaptures = (pawns & ~FILE_A) << 7 & enemies & checkMask;
		Bitboard rightCaptures = (pawns & ~FILE_H) << 9 & enemies & checkMask;

		captureLoop(leftCaptures, 7, 7);
		captureLoop(rightCaptures, 9, 7);

		Bitboard epSquare = epFileMask & RANK_6;

		Bitboard leftEnpassant = (pawns & ~FILE_A) << 7 & epSquare;
		Bitboard rightEnpassant = (pawns & ~FILE_H) << 9 & epSquare;

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
		
		Bitboard singlePushes = pawns >> 8 & empty & checkMask;
		Bitboard doublePushes = ((pawns >> 8) & empty & RANK_6) >> 8 & empty & checkMask;

		pushLoop(singlePushes, -8, 0);
		doublePushLoop(doublePushes, -16);

		Bitboard leftCaptures = (pawns & ~FILE_A) >> 9 & enemies & checkMask;
		Bitboard rightCaptures = (pawns & ~FILE_H) >> 7 & enemies & checkMask;

		captureLoop(leftCaptures, -9, 0);
		captureLoop(rightCaptures, -7, 0);

		Bitboard epSquareMask = epFileMask & RANK_3;

		Bitboard leftEnpassant = (pawns & ~FILE_A) >> 9 & epSquareMask;
		Bitboard rightEnpassant = (pawns & ~FILE_H) >> 7 & epSquareMask;

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

void generateKnightMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask, Bitboard& pinnedPieces) {
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

	Bitboard empty = ~gameState.bitboards[AllIndex];

	Bitboard enemies = (us == White) ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];
	Bitboard knights = (us == White) ? gameState.bitboards[WKnight]	 : gameState.bitboards[BKnight];

	while (knights) {
		const uint8 from = __builtin_ctzll(knights);
		const Bitboard fromBB = (1ULL << from);

		if (pinnedPieces & fromBB) {
			knights &= (knights - 1);
			continue;
		}

		const Bitboard atk = KNIGHT_ATTACK_TABLE[from] & checkMask;

		const Bitboard noncaptures = atk & empty;
		const Bitboard captures = atk & enemies;

		moveLoop(noncaptures, from);
		captureLoop(captures, from);

		knights &= (knights - 1);
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

	Bitboard all = gameState.bitboards[AllIndex];
	Bitboard bishops = us == White ? gameState.bitboards[WBishop] : gameState.bitboards[BBishop];
	Bitboard enemies = us == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	while (bishops) {
		uint8 bishopSq = __builtin_ctzll(bishops);

		for (int8 i = 0; i < 4; i++) {
			uint8 directionIndex = DIAGONAL_RAY_TABLE_INDICIES[i];
			Bitboard diagonalRay = RAY_MASK[bishopSq][directionIndex];

			if (diagonalRay) {
				Bitboard intersectionWithPiece = all & diagonalRay;

				if (intersectionWithPiece) {
					uint8 firstPieceSq;
					bool isDecreasing = DIAGONAL_DECREASES[i]; 

					if (isDecreasing) firstPieceSq = 63 - __builtin_clzll(intersectionWithPiece);
					else firstPieceSq = __builtin_ctzll(intersectionWithPiece);

					Bitboard nonCaptures = RAY_BETWEEN[bishopSq][firstPieceSq] & checkMask;
					moveLoop(nonCaptures, bishopSq);

					if ((1ULL << firstPieceSq) & enemies & checkMask) captureLoop(1ULL << firstPieceSq, bishopSq);
				}
				else moveLoop(diagonalRay & checkMask, bishopSq);
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

	Bitboard all = gameState.bitboards[AllIndex];
	Bitboard rooks = us == White ? gameState.bitboards[WRook] : gameState.bitboards[BRook];
	Bitboard enemies = us == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	while (rooks) {
		uint8 rookSq = __builtin_ctzll(rooks);

		for (int8 i = 0; i < 4; i++) {
	 		uint8 directionIndex = STRAIGHT_RAY_TABLE_INDICIES[i];
			Bitboard straightRay = RAY_MASK[rookSq][directionIndex];

			if (straightRay) {
				Bitboard intersectionWithPiece = all & straightRay;

				if (intersectionWithPiece) {
					bool isDecreasing = STRAIGHT_DECREASES[i]; 
					uint8 firstPieceSq;

					if (isDecreasing) firstPieceSq = 63 - __builtin_clzll(intersectionWithPiece);
					else  firstPieceSq = __builtin_ctzll(intersectionWithPiece);

					Bitboard nonCaptures = RAY_BETWEEN[rookSq][firstPieceSq] & checkMask;
					moveLoop(nonCaptures, rookSq);

					if ((1ULL << firstPieceSq) & enemies & checkMask) captureLoop(1ULL << firstPieceSq, rookSq);
				}
				else moveLoop(straightRay & checkMask, rookSq);
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
				moves.push_back(Move(from, to, CAPTURE_FLAG));
			}

			bb &= bb - 1;
		}
	};

	Bitboard all = gameState.bitboards[AllIndex];
	Bitboard queens = us == White ? gameState.bitboards[WQueen] : gameState.bitboards[BQueen];
	Bitboard enemies = us == White ? gameState.bitboards[BlackIndex] : gameState.bitboards[WhiteIndex];

	while (queens) {
		uint8 queenSq = __builtin_ctzll(queens);

		for (int8 i = 0; i < 8; i++) {
			int8 directionIndex = RAY_TABLE_INDICIES[i];
			Bitboard ray = RAY_MASK[queenSq][directionIndex];

			if (ray) {
				Bitboard intersectionWithPiece = all & ray;

				if (intersectionWithPiece) {
					bool isDecreasing = DIRECTION_DECREASES[i]; 
					uint8 firstPieceSq;

					if (isDecreasing) firstPieceSq = 63 - __builtin_clzll(intersectionWithPiece);
					else firstPieceSq = __builtin_ctzll(intersectionWithPiece);
					
					Bitboard nonCaptures = RAY_BETWEEN[queenSq][firstPieceSq] & checkMask;
					moveLoop(nonCaptures, queenSq);

					if ((1ULL << firstPieceSq) & enemies & checkMask) captureLoop(1ULL << firstPieceSq, queenSq);	
				}
				else moveLoop(ray & checkMask, queenSq);
			}
		}
		queens &= queens - 1;
	}
}

void generateKingMoves(GameState& gameState, std::vector<Move>& moves, Color us, Bitboard& checkMask) {
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
			if (!isSquareAttacked(gameState, 1ULL << 3, them) &&
				!isSquareAttacked(gameState, 1ULL << 2, them) &&
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
			if (!isSquareAttacked(gameState, 1ULL << 59, them) &&
				!isSquareAttacked(gameState, 1ULL << 58, them) &&
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
		generateKingMoves(gameState, moves, us, checkMask);
		return;
	}

	generatePawnMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateKnightMoves(gameState, moves, us, checkMask, pinnedPieces);
	generateBishopMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateRookMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateQueenMoves(gameState, moves, us, checkMask, pinnedPieces, pinnedRays);
	generateKingMoves(gameState, moves, us, checkMask);
}

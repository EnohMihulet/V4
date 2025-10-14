#pragma once
#include <array>
#include <iostream>
#include <ostream>

#include "../chess/Common.h"
// 1, 8, -1, -8, 9, 7, -9, -7
constexpr int8 RIGHT_RAY_TABLE_INDEX = 0;
constexpr int8 UP_RAY_TABLE_INDEX = 1;
constexpr int8 LEFT_RAY_TABLE_INDEX = 2;
constexpr int8 DOWN_RAY_TABLE_INDEX = 3;
constexpr int8 UP_RIGHT_RAY_TABLE_INDEX = 4;
constexpr int8 UP_LEFT_RAY_TABLE_INDEX = 5;
constexpr int8 DOWN_LEFT_RAY_TABLE_INDEX = 6;
constexpr int8 DOWN_RIGHT_RAY_TABLE_INDEX = 7;

constexpr int8 RAY_TABLE_INDICIES[8] = {RIGHT_RAY_TABLE_INDEX, UP_RAY_TABLE_INDEX, LEFT_RAY_TABLE_INDEX, DOWN_RAY_TABLE_INDEX, 
					UP_RIGHT_RAY_TABLE_INDEX, UP_LEFT_RAY_TABLE_INDEX, DOWN_LEFT_RAY_TABLE_INDEX, DOWN_RIGHT_RAY_TABLE_INDEX };
constexpr bool DIRECTION_DECREASES[8] = {false, false, true, true, false, false, true, true};

constexpr int8 DIAGONAL_RAY_TABLE_INDICIES[4] = {UP_RIGHT_RAY_TABLE_INDEX, UP_LEFT_RAY_TABLE_INDEX, DOWN_LEFT_RAY_TABLE_INDEX, DOWN_RIGHT_RAY_TABLE_INDEX};
constexpr bool DIAGONAL_DECREASES[4] = {false, false, true, true};

constexpr int8 STRAIGHT_RAY_TABLE_INDICIES[4] = {RIGHT_RAY_TABLE_INDEX, LEFT_RAY_TABLE_INDEX, DOWN_RAY_TABLE_INDEX, UP_RAY_TABLE_INDEX};
constexpr bool STRAIGHT_DECREASES[4] = {false, true, true, false};

constexpr std::array<Bitboard, 64> generateKnightAttackTable() {
	std::array<Bitboard, 64> t{};
	Bitboard knight = 0;
	for (int i = 0; i < 64; i++) {
		knight = 1ULL << i;

		Bitboard leftUp = (knight & ~(FILE_A | FILE_B | RANK_8)) << 6;
		Bitboard rightUp = (knight & ~(FILE_G | FILE_H | RANK_8)) << 10;
		Bitboard upLeft = (knight & ~(FILE_A | RANK_7 | RANK_8)) << 15;
		Bitboard upRight = (knight & ~(FILE_H | RANK_7 | RANK_8)) << 17;
		Bitboard leftDown = (knight & ~(FILE_A | FILE_B | RANK_1)) >> 10;
		Bitboard rightDown = (knight & ~(FILE_G | FILE_H | RANK_1)) >> 6;
		Bitboard downLeft = (knight & ~(FILE_A | RANK_1 | RANK_2)) >> 17;
		Bitboard downRight = (knight & ~(FILE_H | RANK_1 | RANK_2)) >> 15;

		t[i] = leftUp | rightUp | upLeft | upRight | leftDown | rightDown | downLeft | downRight;
	}
	return t;
}

constexpr std::array<Bitboard, 64> generateKingAttackTable() {
	std::array<Bitboard, 64> t{};
	Bitboard king = 0;

	const int directions[8] = {1, 8, -1, -8, 9, 7, -9, -7};
	const uint64_t boundaryMasks[8] = {FILE_H, RANK_8, FILE_A, RANK_1, FILE_H | RANK_8, FILE_A | RANK_8, FILE_A | RANK_1, FILE_H | RANK_1};

	for (int i = 0; i < 64; i++) {
		king = 1ULL << i;
		t[i] = 0;

		for (int d = 0; d < 8; d++) {
			int direction = directions[d]; 
			uint64_t boundaryMask = boundaryMasks[d]; 
			Bitboard current = direction < 0 ? king >> -direction : king << direction;;

			if (king & boundaryMask) continue;

			t[i] |= current;
		}
	}
	return t;
}

constexpr std::array<std::array<Bitboard, 64>, 2> generatePawnAttackTable() {
	std::array<std::array<Bitboard, 64>, 2> t{}; 
	Bitboard pawn = 0;

	const int directions[4] = {7, 9, -9, -7};
	for (int c = 0; c < 2; c++) {
		int leftShift = directions[c*2];
		int rightShift = directions[c*2 + 1];
		for (int i = 0; i < 64; i++) {
			pawn = 1ULL << i;

			Bitboard leftCapture = (leftShift > 0 ? (pawn & ~FILE_A) << leftShift : (pawn & ~FILE_A) >> -leftShift);
			Bitboard rightCapture = (rightShift > 0 ? (pawn & ~FILE_H) << rightShift : (pawn & ~FILE_H) >> -rightShift);

			t[c][i] = leftCapture | rightCapture;
		}
	}
	
	return t;
}

constexpr std::array<std::array<Bitboard, 64>, 64> generateRayBetweenTable() {
	std::array<std::array<Bitboard, 64>, 64> t{};
	const int directions[8] = {1, 8, -1, -8, 9, 7, -9, -7};
	const uint64_t boundaryMasks[8] = {FILE_H, RANK_8, FILE_A, RANK_1, FILE_H | RANK_8, FILE_A | RANK_8, FILE_A | RANK_1, FILE_H | RANK_1};

	for (int from = 0; from < 64; from++) {
		for (int to = 0; to < 64; to++) {
			if (from == to) {
				t[from][to] = 0;
				continue;
			}

			Bitboard between = 0ULL;
			Bitboard start = 1ULL << from;
			Bitboard target = 1ULL << to;

			for (int d = 0; d < 8; d++) {
				Bitboard current = start;
				Bitboard ray = 0ULL;
				int direction = directions[d];
				uint64_t boundaryMask = boundaryMasks[d];

				while ((current & boundaryMask) == 0) {
					current = direction < 0 ? current >> -direction : current << direction;
					if (current == 0) break;

					ray |= current;
					if (current == target) {
						ray &= ~(start | target);
						between = ray;
						break;
					}
				}
				if (between) break;
			}
			t[from][to] = between;
		}
	}
	return t;
}

constexpr std::array<std::array<Bitboard, 8>, 64> generateRayMaskTable() {
	std::array<std::array<Bitboard, 8>, 64> t{};
	const int directions[8] = {1, 8, -1, -8, 9, 7, -9, -7};
	const uint64_t boundaryMasks[8] = { FILE_H, RANK_8, FILE_A, RANK_1, FILE_H | RANK_8, FILE_A | RANK_8, FILE_A | RANK_1, FILE_H | RANK_1 };

	for (int from = 0; from < 64; from++) {
		Bitboard start = 1ULL << from;

		for (int d = 0; d < 8; d++) {
			Bitboard ray = 0ULL;
			Bitboard current = start;
			int direction = directions[d];
			uint64_t boundaryMask = boundaryMasks[d];

			while ((current & boundaryMask) == 0) {
				current = direction < 0 ? current >> -direction : current << direction;
				if (current == 0) break;
				ray |= current;
			}

			t[from][d] = ray;
		}
	}

	return t;
}


inline constexpr std::array<Bitboard, 64> KNIGHT_ATTACK_TABLE = generateKnightAttackTable();
inline constexpr std::array<Bitboard, 64> KING_ATTACK_TABLE = generateKingAttackTable();
inline constexpr std::array<std::array<Bitboard, 64>, 2> PAWN_ATTACK_TABLE = generatePawnAttackTable();
inline constexpr std::array<std::array<Bitboard, 64>, 64> RAY_BETWEEN = generateRayBetweenTable();
inline constexpr std::array<std::array<Bitboard, 8>, 64> RAY_MASK = generateRayMaskTable();
// TODO: constexpr Bitboard SLIDING_PIECE_ATTACKS[64][8];

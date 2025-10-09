#pragma once
#include <array>

#include "../chess/Common.h"

constexpr uint64 ZOBRIST_SEED = 80087232319ULL;

constexpr uint64 splitMix64(uint64& state) {
	uint64 z = (state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

constexpr std::array<uint64, 768> generatePieceZobristKeys(uint64 state) {
	std::array<uint64, 768> keys{};
	for (auto& k : keys) k = splitMix64(state);
	return keys;
}

constexpr std::array<uint64, 16> generateCastlingZobristKeys(uint64 state) {
	std::array<uint64, 16> keys{};
	for (auto& k : keys) k = splitMix64(state);
	return keys;
}

constexpr std::array<uint64, 8> generateEnPassantZobristKeys(uint64 state) {
	std::array<uint64, 8> keys{};
	for (auto& k : keys) k = splitMix64(state);
	return keys;
}

constexpr uint64 generateBlackZobristKey(uint64 state) {
	return splitMix64(state);
}

inline constexpr std::array<uint64, 768> PIECE_ZOBRIST_KEYS = generatePieceZobristKeys(ZOBRIST_SEED);
inline constexpr std::array<uint64, 16> CASTLING_ZOBRIST_KEYS = generateCastlingZobristKeys(ZOBRIST_SEED + 0xABCDEFULL);
inline constexpr std::array<uint64, 8> ENPASSANT_ZOBRIST_KEYS = generateEnPassantZobristKeys(ZOBRIST_SEED + 0x123456ULL);
inline constexpr uint64 BLACK_ZOBRIST_KEY = generateBlackZobristKey(ZOBRIST_SEED + 0xFEDCBAULL);

#include <vector>

#include "GameRules.h"
#include "Common.h"

bool isInsufficientMaterial(const GameState& gameState) {
	if (__builtin_popcountll(gameState.bitboards[WPawn]) || __builtin_popcountll(gameState.bitboards[WRook]) || __builtin_popcountll(gameState.bitboards[WQueen])) return false;
	if (__builtin_popcountll(gameState.bitboards[BPawn]) || __builtin_popcountll(gameState.bitboards[BRook]) || __builtin_popcountll(gameState.bitboards[BQueen])) return false;

	Bitboard wBishop = gameState.bitboards[WBishop];
	Bitboard bBishop = gameState.bitboards[BBishop];

	uint8 wBishopCount = __builtin_popcountll(wBishop);
	uint8 bBishopCount = __builtin_popcountll(bBishop);
	uint8 wKnightCount = __builtin_popcountll(gameState.bitboards[WKnight]);
	uint8 bKnightCount = __builtin_popcountll(gameState.bitboards[BKnight]);

	if ((wBishopCount >= 1 && wKnightCount >= 1) || (bBishopCount >= 1 && bKnightCount >= 1)) return false;
	if (wKnightCount >= 3 || bKnightCount >= 3) return false;

	uint8 wLSBishopCount = __builtin_popcountll(wBishop & LIGHT_SQUARES);
	uint8 wDSBishopCount = __builtin_popcountll(wBishop & DARK_SQUARES);
	uint8 bLSBishopCount = __builtin_popcountll(bBishop & LIGHT_SQUARES);
	uint8 bDSBishopCount = __builtin_popcountll(bBishop & DARK_SQUARES);

	if ((wLSBishopCount >= 1 && wDSBishopCount >= 1) || (bLSBishopCount >= 1 && bDSBishopCount >= 1)) return false;

	return true;
}

SearchGameResult getSearchGameResult(GameState& gameState, RepetitionTable& repTable, uint16 moveCount) {
	if (gameState.halfMoves >= 50) return Draw;
	if (repTable.isRepeated(gameState.zobristHash)) return Draw;
	if (isInsufficientMaterial(gameState)) return Draw;
	if (moveCount == 0) {
		Bitboard kingPos = gameState.colorToMove == White ? gameState.bitboards[WKing] : gameState.bitboards[BKing];
		bool isCheck = isSquareAttacked(gameState, kingPos, gameState.colorToMove == White ? Black : White);
		if (isCheck) return Checkmate;
		return Draw;
	}
	return NotDone;
}

SearchGameResult getSearchGameResult(GameState& gameState, RepetitionTable& repTable, uint16 moveCount, bool isCheck) {
	if (moveCount == 0) {
		if (isCheck) return Checkmate;
		return Draw;
	}
	if (gameState.halfMoves >= 50) return Draw;
	if (repTable.isRepeated(gameState.zobristHash)) return Draw;
	if (isInsufficientMaterial(gameState)) return Draw;
	return NotDone;
}

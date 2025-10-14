#ifdef DEBUG_EVAL
#include <iostream>
#endif

#include "Evaluation.h"
#include "PieceSquareTables.h"

int16 calculateMgWeight(const GameState& gameState) {
	int16 mgWeight = 0;
	mgWeight += __builtin_popcountll(gameState.bitboards[WKnight]);
	mgWeight += __builtin_popcountll(gameState.bitboards[BKnight]);
	mgWeight += __builtin_popcountll(gameState.bitboards[WBishop]);
	mgWeight += __builtin_popcountll(gameState.bitboards[BBishop]);
	mgWeight += 2*__builtin_popcountll(gameState.bitboards[WRook]);
	mgWeight += 2*__builtin_popcountll(gameState.bitboards[BRook]);
	mgWeight += 4*__builtin_popcountll(gameState.bitboards[WQueen]);
	mgWeight += 4*__builtin_popcountll(gameState.bitboards[BQueen]);

	return mgWeight;
}

int16 evaluate(const GameState& gameState, Color color) {
	int16 mgWeight = calculateMgWeight(gameState);
	int16 egWeight = TOTAL_PHASE - mgWeight;

	float mgFactor = (float) mgWeight / TOTAL_PHASE;
	float egFactor = (float) egWeight / TOTAL_PHASE;


	EvalComponents comps;
	comps.pawns = evaluatePawns(gameState, mgFactor, egFactor);
	comps.knights = evaluateKnights(gameState, mgFactor, egFactor);
	comps.bishops = evaluateBishops(gameState, mgFactor, egFactor);
	comps.rooks = evaluateRooks(gameState, mgFactor, egFactor);
	comps.queens = evaluateQueen(gameState, mgFactor, egFactor);
	comps.kings = evaluateKing(gameState, mgFactor, egFactor);

	int16 score = comps.pawns + comps.knights + comps.bishops + comps.rooks + comps.queens + comps.kings;
	return color == White ? score : -score;
}

int16 evaluate(const GameState& gameState) {
	int16 mgWeight = calculateMgWeight(gameState);
	int16 egWeight = TOTAL_PHASE - mgWeight;

	float mgFactor = (float) mgWeight / TOTAL_PHASE;
	float egFactor = (float) egWeight / TOTAL_PHASE;


	EvalComponents comps;
	comps.pawns = evaluatePawns(gameState, mgFactor, egFactor);
	comps.knights = evaluateKnights(gameState, mgFactor, egFactor);
	comps.bishops = evaluateBishops(gameState, mgFactor, egFactor);
	comps.rooks = evaluateRooks(gameState, mgFactor, egFactor);
	comps.queens = evaluateQueen(gameState, mgFactor, egFactor);
	comps.kings = evaluateKing(gameState, mgFactor, egFactor);

	int16 score = comps.pawns + comps.knights + comps.bishops + comps.rooks + comps.queens + comps.kings;
	return score;
}

int16 evaluatePawns(const GameState& gameState, float mgFactor, float egFactor) {
	// TODO: MAYBE CONNECTED PASSED PAWNS and BACKWARD PAWNS
	Bitboard wPawns = gameState.bitboards[WPawn];
	Bitboard bPawns = gameState.bitboards[BPawn];

	int16 wPieceTableScore = 0;
	int16 bPieceTableScore = 0;
	int16 wPassedPawnScore = 0;
	int16 bPassedPawnScore = 0;
	int16 wIsolatedPawnScore = 0;
	int16 bIsolatedPawnScore = 0;
	int16 wDoubledPawnScore = 0;
	int16 bDoubledPawnScore = 0;
	int16 materialScore = 0;

	Bitboard bb = wPawns;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		uint16 file = sq & 7;
		uint16 rank = sq / 8;

		wPieceTableScore += PAWN_TABLE[sq];

		Bitboard filesMask = FILES[file];
		if (file == 0) filesMask |= FILES[file+1];
		else if (file == 7) filesMask |= FILES[file-1]; 
		else filesMask |= FILES[file-1] | FILES[file+1];

		Bitboard ranksMask = 0;
		for (int i = rank + 1; i <= 7; i++) {
			ranksMask |= RANKS[i];
		}

		if ((bPawns & filesMask & ranksMask) == 0) {
			wPassedPawnScore += egFactor * PASSED_PAWNS[rank];
		}
		if (__builtin_popcountll(wPawns & filesMask) == 1) {
			wIsolatedPawnScore += ISOLATED_PAWNS;
		}
		if (__builtin_popcountll(wPawns & FILES[file]) > 1) {
			wDoubledPawnScore += DOUBLED_PAWNS;
		}
		
		bb &= bb - 1;
	}

	bb = bPawns;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		uint16 file = sq & 7;
		uint16 rank = sq / 8;

		bPieceTableScore += PAWN_TABLE[sq ^ 56];

		Bitboard filesMask = FILES[file];
		if (file == 0) filesMask |= FILES[file+1];
		else if (file == 7) filesMask |= FILES[file-1]; 
		else filesMask |= FILES[file-1] | FILES[file+1];

		Bitboard ranksMask = 0;
		for (int i = rank - 1; i >= 0; i--) {
			ranksMask |= RANKS[i];
		}

		if ((wPawns & filesMask & ranksMask) == 0) {
			bPassedPawnScore += egFactor * PASSED_PAWNS[7 - rank];
		}
		if ((wPawns & (filesMask ^ FILES[file])) == 0) {
			bIsolatedPawnScore += ISOLATED_PAWNS;
		}
		if (__builtin_popcountll(bPawns & FILES[file]) > 1) {
			bDoubledPawnScore += DOUBLED_PAWNS;
		}

		bb &= bb - 1;
	}

	int16 wPawnCount = __builtin_popcountll(wPawns); 
	int16 bPawnCount = __builtin_popcountll(bPawns); 
	materialScore += (wPawnCount - bPawnCount) * PIECE_VALUES[WPawn];

	int16 wScore = wPieceTableScore + wPassedPawnScore + wIsolatedPawnScore + wDoubledPawnScore;
	int16 bScore = bPieceTableScore + bPassedPawnScore + bIsolatedPawnScore + bDoubledPawnScore;
	int16 total = wScore - bScore + materialScore;

	#ifdef DEBUG_EVAL
	std::cout << "Pawn Eval: wPT=" << wPieceTableScore
		<< " wPassed=" << wPassedPawnScore
		<< " wIsolated=" << wIsolatedPawnScore
		<< " wDoubled=" << wDoubledPawnScore
		<< " | bPT=" << bPieceTableScore
		<< " bPassed=" << bPassedPawnScore
		<< " bIsolated=" << bIsolatedPawnScore
		<< " bDoubled=" << bDoubledPawnScore
		<< " | mat=" << materialScore
		<< " | total=" << total
		<< "\n";
	#endif

	return total;
}

int16 evaluateKnights(const GameState& gameState, float mgFactor, float egFactor) {
	// TODO: MOBILITY SCORE
	Bitboard wKnights = gameState.bitboards[WKnight];
	Bitboard bKnights = gameState.bitboards[BKnight];
	
	int16 wPieceTableScore = 0;
	int16 bPieceTableScore = 0;
	int16 materialScore = 0;
	int16 wKnightPairBonus = 0;
	int16 bKnightPairBonus = 0;
	int16 wKnightAdjustment = 0;
	int16 bKnightAdjustment = 0;

	Bitboard bb = wKnights;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		wPieceTableScore += KNIGHT_TABLE[sq];
		bb &= bb - 1;
	}
	bb = bKnights;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		bPieceTableScore += KNIGHT_TABLE[sq^56];
		bb &= bb - 1;
	}

	int16 wKnightCount = __builtin_popcountll(wKnights);
	int16 bKnightCount = __builtin_popcountll(bKnights);
	materialScore = (wKnightCount - bKnightCount) * PIECE_VALUES[WKnight];

	if (wKnightCount > 1) wKnightPairBonus = KNIGHT_PAIR;
	if (bKnightCount > 1) bKnightPairBonus = KNIGHT_PAIR;

	int16 wPawnCount = __builtin_popcountll(gameState.bitboards[WPawn]); 
	int16 bPawnCount = __builtin_popcountll(gameState.bitboards[BPawn]); 
	
	wKnightAdjustment = wKnightCount * KNIGHT_ADJUSTMENT[wPawnCount];
	bKnightAdjustment = bKnightCount * KNIGHT_ADJUSTMENT[bPawnCount];

	int16 wScore = wPieceTableScore + wKnightPairBonus + wKnightAdjustment;
	int16 bScore = bPieceTableScore + bKnightPairBonus + bKnightAdjustment;
	int16 total = wScore - bScore + materialScore;

	#ifdef DEBUG_EVAL
	std::cout << "Knight Eval: wPT=" << wPieceTableScore
		<< " wCount=" << wKnightCount
		<< " | bPT=" << bPieceTableScore
		<< " bCount=" << bKnightCount
		<< " | mat=" << materialScore
		<< " wPair=" << wKnightPairBonus
		<< " bPair=" << bKnightPairBonus
		<< " wAdj=" << wKnightAdjustment
		<< " bAdj=" << bKnightAdjustment
		<< " | total=" << total
		<< "\n";
	#endif

	return total;
}


int16 evaluateBishops(const GameState& gameState, float mgFactor, float egFactor) {
	// TODO: MOBILITY SCORE
	Bitboard wBishops = gameState.bitboards[WBishop];
	Bitboard bBishops = gameState.bitboards[BBishop];
	
	int16 wPieceTableScore = 0;
	int16 bPieceTableScore = 0;
	int16 materialScore = 0;
	int16 wBishopPairBonus = 0;
	int16 bBishopPairBonus = 0;

	Bitboard bb = wBishops;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		wPieceTableScore += BISHOP_TABLE[sq];
		bb &= bb - 1;
	}

	bb = bBishops;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		bPieceTableScore += BISHOP_TABLE[sq ^ 56];
		bb &= bb - 1;
	}

	int16 wBishopCount = __builtin_popcountll(wBishops);
	int16 bBishopCount = __builtin_popcountll(bBishops);
	materialScore = (wBishopCount - bBishopCount) * PIECE_VALUES[WBishop];

	if (wBishopCount > 1) wBishopPairBonus = BISHOP_PAIR;
	if (bBishopCount > 1) bBishopPairBonus = BISHOP_PAIR;

	int16 wScore = wPieceTableScore + wBishopPairBonus;
	int16 bScore = bPieceTableScore + bBishopPairBonus;
	int16 total = wScore - bScore + materialScore;

	#ifdef DEBUG_EVAL
	std::cout << "Bishop Eval: wPT=" << wPieceTableScore
		<< " wCount=" << wBishopCount
		<< " wPair=" << wBishopPairBonus
		<< " | bPT=" << bPieceTableScore
		<< " bCount=" << bBishopCount
		<< " bPair=" << bBishopPairBonus
		<< " | mat=" << materialScore
		<< " | total=" << total << "\n";
	#endif

	return total;
}

int16 evaluateRooks(const GameState& gameState, float mgFactor, float egFactor) {
	// TODO: MOBILITY SCORES
	Bitboard wRooks = gameState.bitboards[WRook];
	Bitboard bRooks = gameState.bitboards[BRook];
	Bitboard wPawns = gameState.bitboards[WPawn];
	Bitboard bPawns = gameState.bitboards[BPawn];

	int16 wPieceTableScore = 0;
	int16 bPieceTableScore = 0;
	int16 materialScore = 0;
	int16 wRookPairBonus = 0;
	int16 bRookPairBonus = 0;
	int16 wOpenFileScore = 0;
	int16 bOpenFileScore = 0;
	int16 wRookAdjustment = 0;
	int16 bRookAdjustment = 0;

	Bitboard bb = wRooks;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		wPieceTableScore += ROOK_TABLE[sq];

		uint16 file = sq & 7;
		Bitboard fileMask = FILES[file];
		uint16 pawnCountInFile = __builtin_popcountll((wPawns | bPawns) & fileMask); 
		uint16 wPawnCountInFile = __builtin_popcountll(wPawns & fileMask); 
		
		if (pawnCountInFile == 0) wOpenFileScore += ROOK_OPEN_FILE;
		else if (wPawnCountInFile == 0) wOpenFileScore += ROOK_SEMI_OPEN_FILE;

		bb &= bb - 1;
	}

	bb = bRooks;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		bPieceTableScore += ROOK_TABLE[sq ^ 56];

		uint16 file = sq & 7;
		Bitboard fileMask = FILES[file];
		uint16 pawnCountInFile = __builtin_popcountll((wPawns | bPawns) & fileMask); 
		uint16 bPawnCountInFile = __builtin_popcountll(bPawns & fileMask); 
		
		if (pawnCountInFile == 0) bOpenFileScore += ROOK_OPEN_FILE;
		else if (bPawnCountInFile == 0) bOpenFileScore += ROOK_SEMI_OPEN_FILE;

		bb &= bb - 1;
	}

	int16 wRookCount = __builtin_popcountll(wRooks);
	int16 bRookCount = __builtin_popcountll(bRooks);
	materialScore = (wRookCount - bRookCount) * PIECE_VALUES[WRook];

	if (wRookCount > 1) wRookPairBonus = ROOK_PAIR;
	if (bRookCount > 1) bRookPairBonus = ROOK_PAIR;

	int16 wPawnCount = __builtin_popcountll(wPawns); 
	int16 bPawnCount = __builtin_popcountll(bPawns); 
	wRookAdjustment = wRookCount * ROOK_ADJUSTMENT[wPawnCount];
	bRookAdjustment = bRookCount * ROOK_ADJUSTMENT[bPawnCount];

	int16 wScore = wPieceTableScore + wRookPairBonus + wOpenFileScore + wRookAdjustment;
	int16 bScore = bPieceTableScore + bRookPairBonus + bOpenFileScore + bRookAdjustment;
	int16 total = wScore - bScore + materialScore;

	#ifdef DEBUG_EVAL
	std::cout << "Rook Eval: wPT=" << wPieceTableScore
		<< " wCount=" << wRookCount
		<< " wPair=" << wRookPairBonus
		<< " wFiles=" << wOpenFileScore
		<< " wAdj=" << wRookAdjustment
		<< " | bPT=" << bPieceTableScore
		<< " bCount=" << bRookCount
		<< " bPair=" << bRookPairBonus
		<< " bFiles=" << bOpenFileScore
		<< " bAdj=" << bRookAdjustment
		<< " | mat=" << materialScore
		<< " | total=" << total << "\n";
	#endif

	return total;
}

int16 evaluateQueen(const GameState& gameState, float mgFactor, float egFactor) {
	// TODO: MOBILITY SCORES
	Bitboard wQueens = gameState.bitboards[WQueen];
	Bitboard bQueens = gameState.bitboards[BQueen];

	int16 wPieceTableScore = 0;
	int16 bPieceTableScore = 0;
	int16 materialScore = 0;

	Bitboard bb = wQueens;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		wPieceTableScore += QUEEN_TABLE[sq];
		bb &= bb - 1;
	}

	bb = bQueens;
	while (bb) {
		uint16 sq = __builtin_ctzll(bb);
		bPieceTableScore += QUEEN_TABLE[sq ^ 56];
		bb &= bb - 1;
	}

	int16 wQueenCount = __builtin_popcountll(wQueens);
	int16 bQueenCount = __builtin_popcountll(bQueens);
	materialScore = (wQueenCount - bQueenCount) * PIECE_VALUES[WQueen];

	int16 wScore = wPieceTableScore;
	int16 bScore = bPieceTableScore;
	int16 total = wScore - bScore + materialScore;

	#ifdef DEBUG_EVAL
	std::cout << "Queen Eval: wPT=" << wPieceTableScore
		<< " wCount=" << wQueenCount
		<< " | bPT=" << bPieceTableScore
		<< " bCount=" << bQueenCount
		<< " | mat=" << materialScore
		<< " | total=" << total << "\n";
	#endif

	return total;
}


int16 evaluateKing(const GameState& gameState, float mgFactor, float egFactor) {
	Bitboard wKing = gameState.bitboards[WKing];
	Bitboard bKing = gameState.bitboards[BKing];
	Bitboard wPawns = gameState.bitboards[WPawn];
	Bitboard bPawns = gameState.bitboards[BPawn];

	int16 wPieceTableScore = 0;
	int16 bPieceTableScore = 0;
	int16 wCastledScore = 0;
	int16 bCastledScore = 0;
	int16 wPawnShieldScore = 0;
	int16 bPawnShieldScore = 0;

	Bitboard bb = wKing;
	uint16 sq;
	if (bb) sq = __builtin_ctzll(bb);
	else return 0;

	uint16 wFile = sq & 7;
	uint16 wRank = sq / 8;
	wPieceTableScore = mgFactor*KING_TABLE_MG[sq] + egFactor*KING_TABLE_EG[sq];

	bb = bKing;
	if (bb) sq = __builtin_ctzll(bb);
	else return 0;

	uint16 bFile = sq & 7;
	uint16 bRank = sq / 8;
	bPieceTableScore = mgFactor*KING_TABLE_MG[sq^56] + egFactor*KING_TABLE_EG[sq^56];

	if (__builtin_popcountll(wKing & RANK_1 & FILE_F & FILE_G & FILE_H) == 1) wCastledScore = mgFactor*CASTLED;
	else if (__builtin_popcountll(wKing & RANK_1 & FILE_A & FILE_B & FILE_C) == 1) wCastledScore = mgFactor*CASTLED;

	if (__builtin_popcountll(bKing & RANK_8 & FILE_F & FILE_G & FILE_H) == 1) bCastledScore = mgFactor*CASTLED;
	else if (__builtin_popcountll(bKing & RANK_8 & FILE_A & FILE_B & FILE_C) == 1) bCastledScore = mgFactor*CASTLED;

	Bitboard filesMask = FILES[wFile];
	if (wFile == 0) filesMask |= FILES[wFile+1];
	else if (wFile == 7) filesMask |= FILES[wFile-1]; 
	else filesMask |= FILES[wFile-1] | FILES[wFile+1];

	Bitboard wPawnMask = 0;
	if (wRank < 7) {
		wPawnMask = RANKS[wRank+1] & filesMask;
		wPawnShieldScore += mgFactor*STRONG_PAWN_SHIELD*__builtin_popcountll(wPawns & wPawnMask);
	}
	if (wRank < 6) {
		wPawnMask = RANKS[wRank+2] & filesMask;
		wPawnShieldScore += mgFactor*MID_PAWN_SHIELD*__builtin_popcountll(wPawns & wPawnMask);
	}

	filesMask = FILES[bFile];
	if (bFile == 0) filesMask |= FILES[bFile+1];
	else if (bFile == 7) filesMask |= FILES[bFile-1]; 
	else filesMask |= FILES[bFile-1] | FILES[bFile+1];

	Bitboard bPawnMask = 0;
	if (bRank > 0) {
		bPawnMask = RANKS[bRank-1] & filesMask;
		bPawnShieldScore += mgFactor*STRONG_PAWN_SHIELD*__builtin_popcountll(bPawns & bPawnMask);
	}
	if (bRank > 1) {
		bPawnMask = RANKS[bRank-2] & filesMask;
		bPawnShieldScore += mgFactor*MID_PAWN_SHIELD*__builtin_popcountll(bPawns & bPawnMask);
	}

	int16 wScore = wPieceTableScore + wCastledScore + wPawnShieldScore;
	int16 bScore = bPieceTableScore + bCastledScore + bPawnShieldScore;
	int16 total = wScore - bScore;

	#ifdef DEBUG_EVAL
	std::cout << "King Eval: wPT=" << wPieceTableScore 
		<< " wCastle=" << wCastledScore 
		<< " wShield=" << wPawnShieldScore 
		<< " | bPT=" << bPieceTableScore 
		<< " bCastle=" << bCastledScore 
		<< " bShield=" << bPawnShieldScore 
		<< " | total=" << total
		<< "\n";
	#endif

	return total;
}

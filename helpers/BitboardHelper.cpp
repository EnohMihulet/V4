#include <iostream>

#include "BitboardHelper.h"

void printBitboard(Bitboard bb) {
    std::cout << "Bitboard: " << bb << std::endl;
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            std::cout << ((bb >> square) & 1ULL ? "1 " : ". ");
        }
        std::cout << " " << (rank + 1) << std::endl;
    }
    std::cout << "a b c d e f g h\n" << std::endl;
}

void printGameStateBitboards(const GameState& state) {
    for (size_t i = 0; i < state.bitboards.size(); ++i) {
        std::cout << "Bitboard[" << i << "]:" << std::endl;
        printBitboard(state.bitboards[i]);
    }
}

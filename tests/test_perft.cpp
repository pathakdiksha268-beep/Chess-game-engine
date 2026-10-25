// Perft: count every position reachable in N moves and compare with the official numbers
// (chessprogramming.org "Perft Results"). If the move generator has even one wrong rule
// (castling, en passant, promotion, pins ...) these counts will not match.
#include <initializer_list>
#include <string>
#include "TestFramework.h"
#include "TestHelpers.h"

static void expectPerft(const std::string& fen, std::initializer_list<long long> expected) {
    int depth = 1;
    for (long long want : expected) {
        Board b;
        setupFen(b, fen);
        const long long got = perft(b, Color::White, depth);
        if (got != want) {
            testing::fail(__FILE__, __LINE__, "depth " + std::to_string(depth) + ": got " + std::to_string(got) +
                                                  ", expected " + std::to_string(want));
        }
        depth++;
    }
}

TEST(Perft, StartingPosition) {
    expectPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", {20, 400, 8902, 197281});
}

TEST(Perft, KiwipeteCastlingAndPins) {
    expectPerft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R", {48, 2039, 97862});
}

TEST(Perft, EnPassantAndRookEndgame) {
    expectPerft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8", {14, 191, 2812, 43238});
}

TEST(Perft, PromotionsAndBlackCastling) {
    expectPerft("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1", {6, 264, 9467});
}

TEST(Perft, PromotionCapturesAndCheckEvasion) {
    expectPerft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R", {44, 1486, 62379});
}

TEST(Perft, MiddlegameWithManyPieces) {
    expectPerft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1", {46, 2079, 89890});
}

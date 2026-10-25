// Check, checkmate, stalemate and the "own king must stay safe" rule.
#include "TestFramework.h"
#include "TestHelpers.h"

TEST(Check, NoCheckAtTheStart) {
    Board b; b.setupStartingPosition();
    CHECK(!b.isInCheck(Color::White) && !b.isInCheck(Color::Black));
}

TEST(Check, RookGivesCheckAlongAFile) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/8/4RK2");
    CHECK(b.isInCheck(Color::Black));
    CHECK(!b.isInCheck(Color::White));
}

TEST(Check, BishopGivesCheckAlongADiagonal) {
    Board b; setupFen(b, "4k3/8/8/7B/8/8/8/4K3");    // bishop h5 sees e8 through g6 and f7
    CHECK(b.isInCheck(Color::Black));
    Board c; setupFen(c, "4k3/8/8/8/8/8/1B6/4K3");   // bishop b2 does not
    CHECK(!c.isInCheck(Color::Black));
}

TEST(Check, KnightGivesCheck) {
    Board b; setupFen(b, "4k3/8/5N2/8/8/8/8/4K3");   // knight f6 attacks e8
    CHECK(b.isInCheck(Color::Black));
}

TEST(Check, PawnChecksDiagonallyNotStraight) {
    Board b; setupFen(b, "4k3/3P4/8/8/8/8/8/4K3");   // white pawn d7 attacks e8
    CHECK(b.isInCheck(Color::Black));
    Board c; setupFen(c, "4k3/4P3/8/8/8/8/8/4K3");   // pawn directly in front gives no check
    CHECK(!c.isInCheck(Color::Black));
}

TEST(Check, BlockedLineGivesNoCheck) {
    Board b; setupFen(b, "4k3/4p3/8/8/8/8/8/4RK2");  // black pawn e7 shields the king
    CHECK(!b.isInCheck(Color::Black));
}

TEST(Check, OnlyMovesThatEscapeCheckAreLegal) {
    // 1.e4 f5 2.d4 Nf6 3.Qh5+ : Black may only capture the queen or block with g6
    Board b; b.setupStartingPosition();
    CHECK(playLegal(b, Color::White, "e2", "e4"));
    CHECK(playLegal(b, Color::Black, "f7", "f5"));
    CHECK(playLegal(b, Color::White, "d2", "d4"));
    CHECK(playLegal(b, Color::Black, "g8", "f6"));
    CHECK(playLegal(b, Color::White, "d1", "h5"));
    CHECK(b.isInCheck(Color::Black));
    auto moves = b.getLegalMoves(Color::Black);
    CHECK_EQ(moves.size(), 2u);
    CHECK(hasMove(moves, "f6", "h5") && hasMove(moves, "g7", "g6"));
    CHECK(!b.isCheckmate(Color::Black));
}

TEST(Check, KingCannotStepIntoCheck) {
    Board b; setupFen(b, "3r3k/8/8/8/8/8/8/4K3");    // black rook d8 covers the d-file
    auto moves = b.getLegalMoves(Color::White);
    CHECK(!hasMove(moves, "e1", "d1") && !hasMove(moves, "e1", "d2"));
    CHECK(hasMove(moves, "e1", "e2") && hasMove(moves, "e1", "f1"));
}

TEST(Check, PinnedPieceCannotLeaveTheLine) {
    Board b; setupFen(b, "k3r3/8/8/8/8/8/4B3/4K3");  // bishop e2 is pinned to the king by the rook
    CHECK_EQ(countFrom(b.getLegalMoves(Color::White), "e2"), 0);
}

TEST(Check, PinnedRookMayMoveAlongThePin) {
    Board b; setupFen(b, "k3r3/8/8/8/8/8/4R3/4K3");  // rook e2 pinned on the e-file
    auto moves = b.getLegalMoves(Color::White);
    CHECK(hasMove(moves, "e2", "e5") && hasMove(moves, "e2", "e8"));   // along the file is fine
    CHECK(!hasMove(moves, "e2", "a2") && !hasMove(moves, "e2", "h2"));
}

TEST(Check, CanCaptureTheCheckingPiece) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/4r3/3QK3");  // rook e2 gives check, the queen can take it
    CHECK(b.isInCheck(Color::White));
    CHECK(hasMove(b.getLegalMoves(Color::White), "d1", "e2"));
}

// ------------------------------------------------------------------- checkmate

TEST(Checkmate, FoolsMate) {
    Board b; b.setupStartingPosition();
    CHECK(playLegal(b, Color::White, "f2", "f3"));
    CHECK(playLegal(b, Color::Black, "e7", "e5"));
    CHECK(playLegal(b, Color::White, "g2", "g4"));
    CHECK(playLegal(b, Color::Black, "d8", "h4"));
    CHECK(b.isInCheck(Color::White));
    CHECK(b.isCheckmate(Color::White));
    CHECK(!b.isStalemate(Color::White));
    CHECK_EQ(b.getLegalMoves(Color::White).size(), 0u);
}

TEST(Checkmate, ScholarsMate) {
    Board b; b.setupStartingPosition();
    CHECK(playLegal(b, Color::White, "e2", "e4"));
    CHECK(playLegal(b, Color::Black, "e7", "e5"));
    CHECK(playLegal(b, Color::White, "f1", "c4"));
    CHECK(playLegal(b, Color::Black, "b8", "c6"));
    CHECK(playLegal(b, Color::White, "d1", "h5"));
    CHECK(playLegal(b, Color::Black, "g8", "f6"));
    CHECK(playLegal(b, Color::White, "h5", "f7"));
    CHECK(b.isCheckmate(Color::Black));
}

TEST(Checkmate, BackRankMate) {
    Board b; setupFen(b, "6k1/5ppp/8/8/8/8/8/4R1K1");
    CHECK(playLegal(b, Color::White, "e1", "e8"));
    CHECK(b.isCheckmate(Color::Black));
}

TEST(Checkmate, CheckThatCanBeAnsweredIsNotMate) {
    Board b; setupFen(b, "6k1/5ppp/2b5/8/8/8/8/4R1K1");   // after Re8+ the bishop on c6 can take the rook
    CHECK(playLegal(b, Color::White, "e1", "e8"));
    CHECK(b.isInCheck(Color::Black));
    CHECK(!b.isCheckmate(Color::Black));
    CHECK(hasMove(b.getLegalMoves(Color::Black), "c6", "e8"));
}

TEST(Checkmate, NotCheckmateAtTheStart) {
    Board b; b.setupStartingPosition();
    CHECK(!b.isCheckmate(Color::White) && !b.isCheckmate(Color::Black));
}

// -------------------------------------------------------------------- stalemate

TEST(Stalemate, KingBoxedInByQueenAndKing) {
    Board b; setupFen(b, "k7/2Q5/1K6/8/8/8/8/8");    // Black Ka8, White Qc7 and Kb6
    CHECK(!b.isInCheck(Color::Black));
    CHECK(b.isStalemate(Color::Black));
    CHECK(!b.isCheckmate(Color::Black));
    CHECK_EQ(b.getLegalMoves(Color::Black).size(), 0u);
}

TEST(Stalemate, NotStalemateWhileAnotherPieceCanMove) {
    Board b; setupFen(b, "k7/2Q4p/1K6/8/8/8/8/8");   // the king is still boxed in, but the h7 pawn can move
    CHECK(!b.isInCheck(Color::Black));
    CHECK(!b.isStalemate(Color::Black));
    CHECK_EQ(b.getLegalMoves(Color::Black).size(), 2u);   // h6 and h5
}

TEST(Stalemate, CheckmateIsNotStalemate) {
    Board b; setupFen(b, "6k1/5ppp/8/8/8/8/8/4R1K1");
    CHECK(playLegal(b, Color::White, "e1", "e8"));
    CHECK(!b.isStalemate(Color::Black));
}

// Castling, en passant and pawn promotion (including undo).
#include "TestFramework.h"
#include "TestHelpers.h"

static int castles(Board& b, Color side) { return countType(b.getLegalMoves(side), MoveType::Castle); }

// ------------------------------------------------------------------------ castling

TEST(Castling, BothSidesAvailableOnAClearBoard) {
    Board b; setupFen(b, "r3k2r/8/8/8/8/8/8/R3K2R");
    CHECK_EQ(castles(b, Color::White), 2);
    CHECK_EQ(castles(b, Color::Black), 2);
}

TEST(Castling, NotAvailableAtTheStart) {
    Board b; b.setupStartingPosition();
    CHECK_EQ(castles(b, Color::White), 0);
}

TEST(Castling, KingsideMovesKingToGAndRookToF) {
    Board b; setupFen(b, "r3k2r/8/8/8/8/8/8/R3K2R");
    CHECK(playLegal(b, Color::White, "e1", "g1"));
    CHECK(typeAt(b, "g1") == PieceType::King && typeAt(b, "f1") == PieceType::Rook);
    CHECK(isEmptyAt(b, "e1") && isEmptyAt(b, "h1"));
}

TEST(Castling, QueensideMovesKingToCAndRookToD) {
    Board b; setupFen(b, "r3k2r/8/8/8/8/8/8/R3K2R");
    CHECK(playLegal(b, Color::White, "e1", "c1"));
    CHECK(typeAt(b, "c1") == PieceType::King && typeAt(b, "d1") == PieceType::Rook);
    CHECK(isEmptyAt(b, "e1") && isEmptyAt(b, "a1"));
}

TEST(Castling, BlackCastlesOnTheTopRow) {
    Board b; setupFen(b, "r3k2r/8/8/8/8/8/8/R3K2R");
    CHECK(playLegal(b, Color::Black, "e8", "g8"));
    CHECK(typeAt(b, "g8") == PieceType::King && typeAt(b, "f8") == PieceType::Rook);
    Board c; setupFen(c, "r3k2r/8/8/8/8/8/8/R3K2R");
    CHECK(playLegal(c, Color::Black, "e8", "c8"));
    CHECK(typeAt(c, "c8") == PieceType::King && typeAt(c, "d8") == PieceType::Rook);
}

TEST(Castling, UndoRestoresKingRookAndTheRightToCastle) {
    Board b; setupFen(b, "r3k2r/8/8/8/8/8/8/R3K2R");
    const std::string before = snapshot(b);
    CHECK(playLegal(b, Color::White, "e1", "g1"));
    b.undoMove();
    CHECK_EQ(snapshot(b), before);
    CHECK_EQ(castles(b, Color::White), 2);          // the "has moved" flags were restored too
}

TEST(Castling, BlockedByAPieceBetweenKingAndRook) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/8/R3K1NR");  // knight on g1
    CHECK_EQ(castles(b, Color::White), 1);           // only queenside is left
    CHECK(!hasMove(b.getLegalMoves(Color::White), "e1", "g1"));
}

TEST(Castling, QueensideNeedsB1ToBeEmptyToo) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/8/RN2K2R");  // knight on b1
    CHECK(!hasMove(b.getLegalMoves(Color::White), "e1", "c1"));
    CHECK(hasMove(b.getLegalMoves(Color::White), "e1", "g1"));
}

TEST(Castling, NotAllowedWhileInCheck) {
    Board b; setupFen(b, "4k3/8/8/4r3/8/8/8/R3K2R");  // black rook gives check along the e-file
    CHECK(b.isInCheck(Color::White));
    CHECK_EQ(castles(b, Color::White), 0);
}

TEST(Castling, NotAllowedThroughAnAttackedSquare) {
    Board b; setupFen(b, "4kr2/8/8/8/8/8/8/R3K2R");  // black rook f8 attacks f1
    auto moves = b.getLegalMoves(Color::White);
    CHECK(!hasMove(moves, "e1", "g1"));              // kingside crosses f1
    CHECK(hasMove(moves, "e1", "c1"));               // queenside is still fine
}

TEST(Castling, NotAllowedOntoAnAttackedSquare) {
    Board b; setupFen(b, "4k1r1/8/8/8/8/8/8/R3K2R"); // black rook g8 attacks g1
    auto moves = b.getLegalMoves(Color::White);
    CHECK(!hasMove(moves, "e1", "g1"));
    CHECK(hasMove(moves, "e1", "c1"));
}

TEST(Castling, AllowedWhenOnlyTheRookIsAttacked) {
    Board b; setupFen(b, "4k2r/8/8/8/8/8/8/R3K2R");  // black rook h8 attacks the white rook on h1
    CHECK_EQ(castles(b, Color::White), 2);
}

TEST(Castling, QueensideAllowedWhenB1IsAttacked) {
    Board b; setupFen(b, "1r2k3/8/8/8/8/8/8/R3K2R"); // b1 is attacked, but the king never crosses it
    CHECK(hasMove(b.getLegalMoves(Color::White), "e1", "c1"));
}

TEST(Castling, NotAllowedAfterTheKingHasMoved) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/8/R3K2R");
    b.makeMove(Move{sq("e1"), sq("e2")});
    b.makeMove(Move{sq("e2"), sq("e1")});            // back on e1, but it has moved
    CHECK_EQ(castles(b, Color::White), 0);
}

TEST(Castling, NotAllowedWithARookThatHasMoved) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/8/R3K2R");
    b.makeMove(Move{sq("h1"), sq("h2")});
    b.makeMove(Move{sq("h2"), sq("h1")});
    auto moves = b.getLegalMoves(Color::White);
    CHECK(!hasMove(moves, "e1", "g1"));              // that rook moved
    CHECK(hasMove(moves, "e1", "c1"));               // the other one did not
}

// ---------------------------------------------------------------------- en passant

// 1.e4 a6 2.e5 d5 : White's e5 pawn may take the d5 pawn "in passing"
static void setupEnPassant(Board& b) {
    b.setupStartingPosition();
    playLegal(b, Color::White, "e2", "e4");
    playLegal(b, Color::Black, "a7", "a6");
    playLegal(b, Color::White, "e4", "e5");
    playLegal(b, Color::Black, "d7", "d5");
}

TEST(EnPassant, AvailableRightAfterTheDoubleStep) {
    Board b; setupEnPassant(b);
    auto moves = b.getLegalMoves(Color::White);
    CHECK(hasMove(moves, "e5", "d6"));
    CHECK_EQ(countType(moves, MoveType::EnPassant), 1);
}

TEST(EnPassant, CapturesThePawnBesideNotOnTheLandingSquare) {
    Board b; setupEnPassant(b);
    CHECK(playLegal(b, Color::White, "e5", "d6"));
    CHECK(typeAt(b, "d6") == PieceType::Pawn && b.getPiece(sq("d6"))->color() == Color::White);
    CHECK(isEmptyAt(b, "d5"));                       // the black pawn is gone
    CHECK(isEmptyAt(b, "e5"));
}

TEST(EnPassant, UndoPutsBothPawnsBack) {
    Board b; setupEnPassant(b);
    const std::string before = snapshot(b);
    CHECK(playLegal(b, Color::White, "e5", "d6"));
    b.undoMove();
    CHECK_EQ(snapshot(b), before);
    CHECK(typeAt(b, "d5") == PieceType::Pawn && b.getPiece(sq("d5"))->color() == Color::Black);
}

TEST(EnPassant, OnlyForOneMove) {
    Board b; setupEnPassant(b);
    CHECK(playLegal(b, Color::White, "h2", "h3"));
    CHECK(playLegal(b, Color::Black, "a6", "a5"));
    CHECK_EQ(countType(b.getLegalMoves(Color::White), MoveType::EnPassant), 0);
}

TEST(EnPassant, NotAfterTwoSingleSteps) {
    Board b; b.setupStartingPosition();
    playLegal(b, Color::White, "e2", "e4");
    playLegal(b, Color::Black, "d7", "d6");
    playLegal(b, Color::White, "e4", "e5");
    playLegal(b, Color::Black, "a7", "a6");
    playLegal(b, Color::White, "h2", "h3");
    playLegal(b, Color::Black, "d6", "d5");          // one step, not two
    CHECK_EQ(countType(b.getLegalMoves(Color::White), MoveType::EnPassant), 0);
}

TEST(EnPassant, BlackCanDoItToo) {
    Board b; b.setupStartingPosition();
    playLegal(b, Color::White, "a2", "a3");
    playLegal(b, Color::Black, "d7", "d5");
    playLegal(b, Color::White, "a3", "a4");
    playLegal(b, Color::Black, "d5", "d4");
    playLegal(b, Color::White, "e2", "e4");          // double step beside the black pawn
    auto moves = b.getLegalMoves(Color::Black);
    CHECK(hasMove(moves, "d4", "e3"));
    CHECK(playLegal(b, Color::Black, "d4", "e3"));
    CHECK(isEmptyAt(b, "e4"));                       // the white pawn is removed
}

TEST(EnPassant, NotAllowedWhenItWouldExposeTheKing) {
    // White king a5 and pawn b5, Black rook h5 and pawn c7: after ...c5, bxc6 e.p. would
    // remove both pawns from the fifth rank and leave the king facing the rook
    Board b; setupFen(b, "7k/2p5/8/KP5r/8/8/8/8");
    CHECK(playLegal(b, Color::Black, "c7", "c5"));
    CHECK(!hasMove(b.getLegalMoves(Color::White), "b5", "c6"));
}

// ---------------------------------------------------------------------- promotion

TEST(Promotion, PawnOnTheSeventhHasFourChoices) {
    Board b; setupFen(b, "7k/P7/8/8/8/8/8/4K3");
    auto moves = b.getLegalMoves(Color::White);
    CHECK_EQ(countFrom(moves, "a7"), 4);
    CHECK_EQ(countType(moves, MoveType::Promotion), 4);
}

TEST(Promotion, CapturingOntoTheLastRowAlsoPromotes) {
    Board b; setupFen(b, "1n5k/P7/8/8/8/8/8/4K3");
    CHECK_EQ(countFrom(b.getLegalMoves(Color::White), "a7"), 8);    // 4 pushes + 4 captures
}

TEST(Promotion, QueenAppearsAndTheMovedFlagIsSet) {
    Board b; setupFen(b, "7k/P7/8/8/8/8/8/4K3");
    CHECK(playLegal(b, Color::White, "a7", "a8", 'q'));
    CHECK(typeAt(b, "a8") == PieceType::Queen && b.getPiece(sq("a8"))->color() == Color::White);
    CHECK(isEmptyAt(b, "a7"));
}

TEST(Promotion, UnderPromotionToEachPiece) {
    const char letters[] = {'r', 'b', 'n'};
    const PieceType expected[] = {PieceType::Rook, PieceType::Bishop, PieceType::Knight};
    for (int i = 0; i < 3; ++i) {
        Board b; setupFen(b, "7k/P7/8/8/8/8/8/4K3");
        CHECK(playLegal(b, Color::White, "a7", "a8", letters[i]));
        CHECK(typeAt(b, "a8") == expected[i]);
    }
}

TEST(Promotion, UndoBringsTheCapturedPieceAndThePawnBack) {
    Board b; setupFen(b, "1n5k/P7/8/8/8/8/8/4K3");
    const std::string before = snapshot(b);
    CHECK(playLegal(b, Color::White, "a7", "b8", 'q'));
    CHECK(typeAt(b, "b8") == PieceType::Queen);
    b.undoMove();
    CHECK_EQ(snapshot(b), before);
    CHECK(typeAt(b, "a7") == PieceType::Pawn && typeAt(b, "b8") == PieceType::Knight);
}

TEST(Promotion, BlackPromotesOnTheFirstRow) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/p7/4K3");
    CHECK_EQ(countFrom(b.getLegalMoves(Color::Black), "a2"), 4);
    CHECK(playLegal(b, Color::Black, "a2", "a1", 'q'));
    CHECK(typeAt(b, "a1") == PieceType::Queen && b.getPiece(sq("a1"))->color() == Color::Black);
}

TEST(Promotion, PromotedQueenMovesLikeAQueen) {
    Board b; setupFen(b, "7k/P7/8/8/8/8/8/4K3");
    CHECK(playLegal(b, Color::White, "a7", "a8", 'q'));
    CHECK(b.isInCheck(Color::Black));                // the new queen checks along the eighth rank
}

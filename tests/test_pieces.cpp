// How each piece moves (its own rules, before the "king must not be in check" filter),
// plus make / undo on the board.
#include "TestFramework.h"
#include "TestHelpers.h"

// ---------------------------------------------------------------- starting position

TEST(StartingPosition, WhiteHasTwentyMoves) {
    Board b;
    b.setupStartingPosition();
    CHECK_EQ(b.getLegalMoves(Color::White).size(), 20u);
}

TEST(StartingPosition, BlackHasTwentyMoves) {
    Board b;
    b.setupStartingPosition();
    CHECK_EQ(b.getLegalMoves(Color::Black).size(), 20u);
}

TEST(StartingPosition, ThirtyTwoPiecesInTheRightPlaces) {
    Board b;
    b.setupStartingPosition();
    int count = 0;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            if (!b.isEmpty({r, c})) count++;
    CHECK_EQ(count, 32);
    CHECK(typeAt(b, "e1") == PieceType::King && b.getPiece(sq("e1"))->color() == Color::White);
    CHECK(typeAt(b, "d8") == PieceType::Queen && b.getPiece(sq("d8"))->color() == Color::Black);
    CHECK(typeAt(b, "a2") == PieceType::Pawn && typeAt(b, "h7") == PieceType::Pawn);
}

// ------------------------------------------------------------------------ knight

TEST(Knight, CentreHasEightMoves) {
    Board b; b.clear(); put(b, "d4", 'N');
    CHECK_EQ(pseudoMoves(b, "d4").size(), 8u);
}

TEST(Knight, CornerHasTwoMoves) {
    Board b; b.clear(); put(b, "a1", 'N');
    CHECK_EQ(pseudoMoves(b, "a1").size(), 2u);
}

TEST(Knight, JumpsOverPieces) {
    Board b; setupFen(b, "8/8/8/8/8/PPP5/PNP5/PPP5");   // knight b2 walled in by its own pawns
    CHECK(hasMove(pseudoMoves(b, "b2"), "b2", "d3"));
}

TEST(Knight, CannotLandOnOwnPieceButCanCapture) {
    Board b; b.clear(); put(b, "d4", 'N'); put(b, "e6", 'P'); put(b, "f5", 'p');
    auto moves = pseudoMoves(b, "d4");
    CHECK(!hasMove(moves, "d4", "e6"));
    CHECK(hasMove(moves, "d4", "f5"));
}

// -------------------------------------------------------------------------- king

TEST(King, CentreHasEightMoves) {
    Board b; b.clear(); put(b, "e4", 'K');
    CHECK_EQ(pseudoMoves(b, "e4").size(), 8u);
}

TEST(King, CornerHasThreeMoves) {
    Board b; b.clear(); put(b, "h8", 'K');
    CHECK_EQ(pseudoMoves(b, "h8").size(), 3u);
}

TEST(King, CannotCaptureOwnPiece) {
    Board b; b.clear(); put(b, "e4", 'K'); put(b, "e5", 'P');
    CHECK(!hasMove(pseudoMoves(b, "e4"), "e4", "e5"));
}

// -------------------------------------------------------------------------- rook

TEST(Rook, EmptyBoardHasFourteenMoves) {
    Board b; b.clear(); put(b, "d4", 'R');
    CHECK_EQ(pseudoMoves(b, "d4").size(), 14u);
}

TEST(Rook, StopsBeforeOwnPiece) {
    Board b; b.clear(); put(b, "d4", 'R'); put(b, "d6", 'P');
    auto moves = pseudoMoves(b, "d4");
    CHECK_EQ(moves.size(), 11u);                  // 3 left + 4 right + 3 down + 1 up (d5)
    CHECK(hasMove(moves, "d4", "d5"));
    CHECK(!hasMove(moves, "d4", "d6"));
}

TEST(Rook, CapturesEnemyAndStops) {
    Board b; b.clear(); put(b, "d4", 'R'); put(b, "d6", 'p');
    auto moves = pseudoMoves(b, "d4");
    CHECK_EQ(moves.size(), 12u);
    CHECK(hasMove(moves, "d4", "d6"));
    CHECK(!hasMove(moves, "d4", "d7"));
}

// ------------------------------------------------------------------------ bishop

TEST(Bishop, CentreHasThirteenMoves) {
    Board b; b.clear(); put(b, "d4", 'B');
    CHECK_EQ(pseudoMoves(b, "d4").size(), 13u);
}

TEST(Bishop, CornerHasSevenMoves) {
    Board b; b.clear(); put(b, "a1", 'B');
    CHECK_EQ(pseudoMoves(b, "a1").size(), 7u);
}

TEST(Bishop, BlockedByOwnPiece) {
    Board b; b.clear(); put(b, "a1", 'B'); put(b, "c3", 'P');
    CHECK_EQ(pseudoMoves(b, "a1").size(), 1u);    // only b2
}

// ------------------------------------------------------------------------- queen

TEST(Queen, CentreHasTwentySevenMoves) {
    Board b; b.clear(); put(b, "d4", 'Q');
    CHECK_EQ(pseudoMoves(b, "d4").size(), 27u);
}

TEST(Queen, MovesLikeRookAndBishopTogether) {
    Board b; b.clear(); put(b, "d4", 'Q');
    auto moves = pseudoMoves(b, "d4");
    CHECK(hasMove(moves, "d4", "d8") && hasMove(moves, "d4", "a4") && hasMove(moves, "d4", "g7") && hasMove(moves, "d4", "a1"));
    CHECK(!hasMove(moves, "d4", "e6"));           // a knight's jump is not a queen move
}

// -------------------------------------------------------------------------- pawn

TEST(Pawn, StartingPawnHasTwoMoves) {
    Board b; b.clear(); put(b, "e2", 'P');
    auto moves = pseudoMoves(b, "e2");
    CHECK_EQ(moves.size(), 2u);
    CHECK(hasMove(moves, "e2", "e3") && hasMove(moves, "e2", "e4"));
}

TEST(Pawn, MovedPawnHasOneMove) {
    Board b; b.clear(); put(b, "e3", 'P');
    CHECK_EQ(pseudoMoves(b, "e3").size(), 1u);
}

TEST(Pawn, BlockedPawnCannotMoveForwardOrCaptureStraight) {
    Board b; b.clear(); put(b, "e2", 'P'); put(b, "e3", 'p');
    CHECK_EQ(pseudoMoves(b, "e2").size(), 0u);
}

TEST(Pawn, CannotJumpOverAPieceWithTheDoubleStep) {
    Board b; b.clear(); put(b, "e2", 'P'); put(b, "e3", 'n');
    CHECK(!hasMove(pseudoMoves(b, "e2"), "e2", "e4"));
}

TEST(Pawn, DoubleStepBlockedOnTheSecondSquare) {
    Board b; b.clear(); put(b, "e2", 'P'); put(b, "e4", 'n');
    auto moves = pseudoMoves(b, "e2");
    CHECK(hasMove(moves, "e2", "e3"));
    CHECK(!hasMove(moves, "e2", "e4"));
}

TEST(Pawn, CapturesDiagonallyOnly) {
    Board b; b.clear(); put(b, "e2", 'P'); put(b, "d3", 'p'); put(b, "f3", 'p');
    auto moves = pseudoMoves(b, "e2");
    CHECK_EQ(moves.size(), 4u);                    // e3, e4, xd3, xf3
    CHECK(hasMove(moves, "e2", "d3") && hasMove(moves, "e2", "f3"));
}

TEST(Pawn, DoesNotCaptureItsOwnPieces) {
    Board b; b.clear(); put(b, "e2", 'P'); put(b, "d3", 'P');
    CHECK(!hasMove(pseudoMoves(b, "e2"), "e2", "d3"));
}

TEST(Pawn, BlackPawnsMoveDownTheBoard) {
    Board b; b.clear(); put(b, "e7", 'p');
    auto moves = pseudoMoves(b, "e7");
    CHECK(hasMove(moves, "e7", "e6") && hasMove(moves, "e7", "e5"));
    CHECK_EQ(moves.size(), 2u);
}

TEST(Pawn, EdgePawnCapturesOnlyOneWay) {
    Board b; b.clear(); put(b, "a4", 'P'); put(b, "b5", 'p');
    CHECK_EQ(pseudoMoves(b, "a4").size(), 2u);     // a5 and xb5
}

// ------------------------------------------------------------------- make / undo

TEST(Board, MakeThenUndoRestoresThePosition) {
    Board b; b.setupStartingPosition();
    const std::string before = snapshot(b);
    b.makeMove(Move{sq("e2"), sq("e4")});
    CHECK(snapshot(b) != before);
    b.undoMove();
    CHECK_EQ(snapshot(b), before);
}

TEST(Board, UndoRestoresTheMovedFlag) {
    Board b; b.setupStartingPosition();
    b.makeMove(Move{sq("e2"), sq("e4")});
    CHECK(b.getPiece(sq("e4"))->hasMoved());
    b.undoMove();
    CHECK(!b.getPiece(sq("e2"))->hasMoved());
}

TEST(Board, CaptureThenUndoBringsTheCapturedPieceBack) {
    Board b; b.clear(); put(b, "d4", 'R'); put(b, "d7", 'p');
    b.makeMove(Move{sq("d4"), sq("d7")});
    CHECK(typeAt(b, "d7") == PieceType::Rook && isEmptyAt(b, "d4"));
    b.undoMove();
    CHECK(typeAt(b, "d7") == PieceType::Pawn && typeAt(b, "d4") == PieceType::Rook);
    CHECK(b.getPiece(sq("d7"))->color() == Color::Black);
}

TEST(Board, ManyMovesUndoneInReverseRestoreTheStart) {
    Board b; b.setupStartingPosition();
    const std::string before = snapshot(b);
    int played = 0;
    Color side = Color::White;
    for (int i = 0; i < 12; ++i) {                 // play the first legal move 12 times
        auto moves = b.getLegalMoves(side);
        if (moves.empty()) break;
        b.makeMove(moves.front());
        side = other(side);
        played++;
    }
    for (int i = 0; i < played; ++i) b.undoMove();
    CHECK_EQ(snapshot(b), before);
}

TEST(Board, UndoWithNoHistoryDoesNothing) {
    Board b; b.setupStartingPosition();
    const std::string before = snapshot(b);
    b.undoMove();
    CHECK_EQ(snapshot(b), before);
}

TEST(Board, SquareNamesMapToTheRightRowAndColumn) {
    CHECK(sq("a8") == (Position{0, 0}));
    CHECK(sq("h1") == (Position{7, 7}));
    CHECK(sq("e2") == (Position{6, 4}));
    Board b; b.setupStartingPosition();
    CHECK(b.getPiece(sq("e2")) != nullptr && b.getPiece(sq("e2"))->type() == PieceType::Pawn);
}

TEST(Board, GetPieceOutsideTheBoardIsNull) {
    Board b; b.setupStartingPosition();
    CHECK(b.getPiece({-1, 0}) == nullptr && b.getPiece({8, 3}) == nullptr && b.getPiece({3, 9}) == nullptr);
}

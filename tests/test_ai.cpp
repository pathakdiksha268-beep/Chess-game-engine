// The evaluation matrices and the minimax search.
#include "TestFramework.h"
#include "TestHelpers.h"
#include "analyzer/Evaluator.h"
#include "analyzer/MinimaxAI.h"

// ------------------------------------------------------------------- evaluator

TEST(Evaluator, StartingPositionIsEqual) {
    Board b; b.setupStartingPosition();
    Evaluator e;
    CHECK_EQ(e.evaluate(b), 0);
}

TEST(Evaluator, ExtraQueenFavoursWhite) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/8/3QK3");
    Evaluator e;
    CHECK(e.evaluate(b) > 800);
}

TEST(Evaluator, ExtraRookFavoursBlack) {
    Board b; setupFen(b, "r3k3/8/8/8/8/8/8/4K3");
    Evaluator e;
    CHECK(e.evaluate(b) < -400);
}

TEST(Evaluator, MaterialOrderIsPawnKnightBishopRookQueen) {
    Evaluator e;
    int score[5];
    const char* pieces = "pnbrq";
    for (int i = 0; i < 5; ++i) {
        Board b; b.clear(); put(b, "e1", 'K'); put(b, "e8", 'k'); put(b, "a2", static_cast<char>(std::toupper(pieces[i])));
        score[i] = e.evaluate(b);
    }
    CHECK(score[0] < score[1] && score[1] < score[3] && score[3] < score[4]);
    CHECK(score[2] >= score[1]);                      // a bishop is worth at least a knight
}

TEST(Evaluator, CentralKnightBeatsCornerKnight) {
    Evaluator e;
    Board centre; setupFen(centre, "4k3/8/8/8/3N4/8/8/4K3");
    Board corner; setupFen(corner, "4k3/8/8/8/8/8/8/N3K3");
    CHECK(e.evaluate(centre) > e.evaluate(corner));
}

TEST(Evaluator, AdvancedPawnBeatsHomePawn) {
    Evaluator e;
    Board advanced; setupFen(advanced, "4k3/8/4P3/8/8/8/8/4K3");
    Board home;     setupFen(home,     "4k3/8/8/8/8/8/4P3/4K3");
    CHECK(e.evaluate(advanced) > e.evaluate(home));
}

TEST(Evaluator, MirroredPositionHasTheOppositeScore) {
    // swapping the colours and turning the board over must flip the sign exactly
    const std::string fens[] = {
        "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/2N2N2/PPPP1PPP/R1BQK2R",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8",
    };
    Evaluator e;
    for (const std::string& fen : fens) {
        Board a; setupFen(a, fen);
        Board m; setupFen(m, mirrorFen(fen));
        CHECK_EQ(e.evaluate(m), -e.evaluate(a));
    }
}

// ---------------------------------------------------------------------- minimax

TEST(Minimax, FindsMateInOneAsBlack) {              // fool's mate: ...Qh4#
    Board b; b.setupStartingPosition();
    playLegal(b, Color::White, "f2", "f3");
    playLegal(b, Color::Black, "e7", "e5");
    playLegal(b, Color::White, "g2", "g4");
    for (int depth = 1; depth <= 3; ++depth) {
        MinimaxAI ai(depth);
        Move m = ai.findBestMove(b, Color::Black);
        CHECK(m.from == sq("d8") && m.to == sq("h4"));
    }
}

TEST(Minimax, FindsBackRankMateAsWhite) {
    Board b; setupFen(b, "6k1/5ppp/8/8/8/8/8/4R1K1");
    MinimaxAI ai(2);
    Move m = ai.findBestMove(b, Color::White);
    CHECK(m.from == sq("e1") && m.to == sq("e8"));
}

TEST(Minimax, TakesAFreeQueen) {
    Board b; setupFen(b, "3q2k1/8/8/8/8/8/8/3R2K1");   // Rxd8+ wins the queen outright
    for (int depth = 1; depth <= 3; ++depth) {
        MinimaxAI ai(depth);
        Move m = ai.findBestMove(b, Color::White);
        CHECK(m.from == sq("d1") && m.to == sq("d8"));
    }
}

TEST(Minimax, BlackAlsoTakesAFreeQueen) {
    Board b; setupFen(b, "3r2k1/8/8/8/8/8/8/3Q2K1");   // the mirror image: ...Rxd1+
    MinimaxAI ai(2);
    Move m = ai.findBestMove(b, Color::Black);
    CHECK(m.from == sq("d8") && m.to == sq("d1"));
}

TEST(Minimax, DoesNotLeaveTheQueenHanging) {
    // White's queen is attacked by a pawn. Any sensible depth-2 move saves it.
    Board b; setupFen(b, "4k3/8/8/8/8/2p5/3Q4/4K3");   // black pawn c3 attacks... d2, so Qd2 is attacked
    MinimaxAI ai(2);
    Move m = ai.findBestMove(b, Color::White);
    CHECK(m.from == sq("d2") && m.to == sq("c3"));      // the best answer is simply to capture the pawn
}

TEST(Minimax, ReturnsALegalMove) {
    Board b; b.setupStartingPosition();
    MinimaxAI ai(2);
    Move m = ai.findBestMove(b, Color::White);
    CHECK(hasMove(b.getLegalMoves(Color::White), "e2", "e2") == false);
    bool legal = false;
    for (const Move& l : b.getLegalMoves(Color::White))
        if (l.from == m.from && l.to == m.to) legal = true;
    CHECK(legal);
}

TEST(Minimax, SearchLeavesTheBoardUntouched) {
    Board b; b.setupStartingPosition();
    playLegal(b, Color::White, "e2", "e4");
    playLegal(b, Color::Black, "e7", "e5");
    const std::string before = snapshot(b);
    const std::size_t moves = b.getLegalMoves(Color::White).size();
    MinimaxAI ai(3);
    ai.findBestMove(b, Color::White);
    CHECK_EQ(snapshot(b), before);
    CHECK_EQ(b.getLegalMoves(Color::White).size(), moves);
}

TEST(Minimax, WorksWithSpecialMovesInTheTree) {
    // castling, en passant and promotion all appear in this position's search tree
    Board b; setupFen(b, "r3k2r/P6p/8/8/8/8/8/R3K2R");
    const std::string before = snapshot(b);
    MinimaxAI ai(3);
    Move m = ai.findBestMove(b, Color::White);
    bool legal = false;
    for (const Move& l : b.getLegalMoves(Color::White))
        if (l.from == m.from && l.to == m.to) legal = true;
    CHECK(legal);
    CHECK_EQ(snapshot(b), before);
}

// ---------------------------------------------------- ranking (used by hints and analysis)

static bool sameMove(const Move& a, const Move& b) {
    return a.from == b.from && a.to == b.to && a.type == b.type && a.promotion == b.promotion;
}

TEST(Ranking, ListsEveryLegalMoveExactlyOnce) {
    Board b; b.setupStartingPosition();
    MinimaxAI ai(2);
    auto ranked = ai.rankMoves(b, Color::White);
    CHECK_EQ(ranked.size(), 20u);
    for (const Move& legal : b.getLegalMoves(Color::White)) {
        int found = 0;
        for (const ScoredMove& r : ranked) if (sameMove(r.move, legal)) found++;
        CHECK_EQ(found, 1);
    }
}

TEST(Ranking, TopMoveIsTheMoveFindBestMoveChooses) {
    const std::string fens[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
        "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/2N2N2/PPPP1PPP/R1BQK2R",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R",
        "6k1/5ppp/8/8/8/8/8/4R1K1",
    };
    for (const std::string& fen : fens) {
        for (Color side : {Color::White, Color::Black}) {
            Board b; setupFen(b, fen);
            MinimaxAI ai(2);
            auto ranked = ai.rankMoves(b, side);
            if (ranked.empty()) continue;
            CHECK(sameMove(ranked.front().move, ai.findBestMove(b, side)));
        }
    }
}

TEST(Ranking, WhiteSeesTheHighestScoreFirst) {
    Board b; setupFen(b, "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
    MinimaxAI ai(2);
    auto ranked = ai.rankMoves(b, Color::White);
    for (std::size_t i = 1; i < ranked.size(); ++i) CHECK(ranked[i - 1].score >= ranked[i].score);
}

TEST(Ranking, BlackSeesTheLowestScoreFirst) {
    Board b; setupFen(b, "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R");
    MinimaxAI ai(2);
    auto ranked = ai.rankMoves(b, Color::Black);
    for (std::size_t i = 1; i < ranked.size(); ++i) CHECK(ranked[i - 1].score <= ranked[i].score);
}

TEST(Ranking, ScoresAreAlwaysFromWhitesPointOfView) {
    Board b; setupFen(b, "4k3/8/8/8/8/8/8/3QK3");   // White has an extra queen
    MinimaxAI ai(2);
    CHECK(ai.rankMoves(b, Color::White).front().score > 800);
    CHECK(ai.rankMoves(b, Color::Black).front().score > 800);   // Black to move, but White is still ahead
}

TEST(Ranking, MateInOneGetsAMateScore) {
    Board b; setupFen(b, "6k1/5ppp/8/8/8/8/8/4R1K1");
    MinimaxAI ai(2);
    auto ranked = ai.rankMoves(b, Color::White);
    CHECK(ranked.front().move.from == sq("e1") && ranked.front().move.to == sq("e8"));
    CHECK(ranked.front().score >= 100000);
    CHECK(ranked[1].score < 100000);                 // no other move mates
}

TEST(Ranking, LeavesTheBoardUntouched) {
    Board b; b.setupStartingPosition();
    playLegal(b, Color::White, "e2", "e4");
    const std::string before = snapshot(b);
    MinimaxAI ai(3);
    ai.rankMoves(b, Color::Black);
    CHECK_EQ(snapshot(b), before);
}

TEST(Ranking, NoMovesMeansAnEmptyList) {
    Board b; b.setupStartingPosition();
    playLegal(b, Color::White, "f2", "f3");
    playLegal(b, Color::Black, "e7", "e5");
    playLegal(b, Color::White, "g2", "g4");
    playLegal(b, Color::Black, "d8", "h4");           // White is checkmated
    MinimaxAI ai(2);
    CHECK(ai.rankMoves(b, Color::White).empty());
}

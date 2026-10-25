#pragma once
#include "engine/Board.h"
#include "Evaluator.h"

// A move together with the score the search gave it (positive = good for White)
struct ScoredMove {
    Move move;
    int score;
};

class MinimaxAI {
public:
    explicit MinimaxAI(int depth = 2) : depth_(depth) {}
    void setDepth(int depth) { depth_ = depth; }

    Move findBestMove(Board& board, Color color);

    // Every legal move with its score, best first for `color`. The first entry is the
    // move findBestMove would choose. Used for hints and analysis.
    std::vector<ScoredMove> rankMoves(Board& board, Color color);

private:
    int minimax(Board& board, int depth, int alpha, int beta, Color toMove);

    int depth_;
    Evaluator evaluator_;
};

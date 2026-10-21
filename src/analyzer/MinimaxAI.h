#pragma once
#include "engine/Board.h"
#include "Evaluator.h"

class MinimaxAI{
public:
    explicit MinimaxAI(int depth = 2) : depth_(depth) {}
    void setDepth(int depth) { depth_ = depth; }

    // Caller must make sure the player has at least one legal move
    Move findBestMove(Board& board, Color color);

private:
    int minimax(Board& board, int depth, int alpha, int beta, Color toMove);

    int depth_;
    Evaluator evaluator_;
};
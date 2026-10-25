#include "MinimaxAI.h"
#include <algorithm>

namespace {
const int INF = 1000000;
const int MATE = 100000;
}

int MinimaxAI::minimax(Board& board, int depth, int alpha, int beta, Color toMove) {
    std::vector<Move> moves = board.getLegalMoves(toMove);

    if (moves.empty()) {
        if (board.isInCheck(toMove)) {
            return (toMove == Color::White) ? -(MATE + depth) : (MATE + depth);
        }
        return 0;
    }
    if (depth == 0) return evaluator_.evaluate(board);

    Color next = (toMove == Color::White) ? Color::Black : Color::White;

    if (toMove == Color::White) {
        int best = -INF;
        for (const Move& m : moves) {
            board.makeMove(m);
            int score = minimax(board, depth - 1, alpha, beta, next);
            board.undoMove();

            if (score > best) best = score;
            if (best > alpha) alpha = best;
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = INF;
        for (const Move& m : moves) {
            board.makeMove(m);
            int score = minimax(board, depth - 1, alpha, beta, next);
            board.undoMove();

            if (score < best) best = score;
            if (best < beta) beta = best;
            if (beta <= alpha) break;
        }
        return best;
    }
}

Move MinimaxAI::findBestMove(Board& board, Color color) {
    std::vector<Move> moves = board.getLegalMoves(color);
    Color other = (color == Color::White) ? Color::Black : Color::White;
    int searchDepth = (depth_ < 1) ? 1 : depth_;

    Move bestMove = moves[0];
    int bestScore = (color == Color::White) ? -INF : INF;

    for (const Move& m : moves) {
        board.makeMove(m);
        int score = minimax(board, searchDepth - 1, -INF, INF, other);
        board.undoMove();

        bool better = (color == Color::White) ? (score > bestScore) : (score < bestScore);
        if (better) {
            bestScore = score;
            bestMove = m;
        }
    }
    return bestMove;
}

std::vector<ScoredMove> MinimaxAI::rankMoves(Board& board, Color color) {
    std::vector<ScoredMove> ranked;
    Color other = (color == Color::White) ? Color::Black : Color::White;
    int searchDepth = (depth_ < 1) ? 1 : depth_;

    for (const Move& m : board.getLegalMoves(color)) {
        board.makeMove(m);
        int score = minimax(board, searchDepth - 1, -INF, INF, other);   // full window: an exact score
        board.undoMove();
        ranked.push_back({m, score});
    }

    // best first; stable, so equal scores keep the order findBestMove uses
    std::stable_sort(ranked.begin(), ranked.end(), [color](const ScoredMove& a, const ScoredMove& b) {
        return color == Color::White ? a.score > b.score : a.score < b.score;
    });
    return ranked;
}

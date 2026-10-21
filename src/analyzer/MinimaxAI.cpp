#include "MinimaxAI.h"
using namespace std;
namespace {
const int INF = 1000000;
const int MATE = 100000;
}

int MinimaxAI::minimax(Board& board, int depth, int alpha, int beta, Color toMove) {
    vector<Move> moves = board.getLegalMoves(toMove);

    // No legal moves: checkmate (very bad for the player to move) or stalemate (draw).
    // Adding depth prefers a quicker mate.
    if (moves.empty()) {
        if (board.isInCheck(toMove)) {
            return (toMove == Color::White) ? -(MATE + depth) : (MATE + depth);
        }
        return 0;
    }
    if (depth == 0) return evaluator_.evaluate(board);

    Color next = (toMove == Color::White) ? Color::Black : Color::White;

    if(toMove == Color::White){                 // White maximizes
        int best = -INF;
        for (const Move& m : moves){
            board.makeMove(m);
            int score = minimax(board, depth - 1, alpha, beta, next);
            board.undoMove();

            if (score > best) best = score;
            if (best > alpha) alpha = best;
            if (beta <= alpha) break;      // prune
        }
        return best;
    } 
    else{                                      // Black minimizes
        int best = INF;
        for (const Move& m : moves) {
            board.makeMove(m);
            int score = minimax(board, depth - 1, alpha, beta, next);
            board.undoMove();
            if (score < best) best = score;
            if (best < beta) beta = best;
            if (beta <= alpha) break;      // prune
        }
        return best;
    }
}

Move MinimaxAI::findBestMove(Board& board, Color color) {
    vector<Move> moves = board.getLegalMoves(color);
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
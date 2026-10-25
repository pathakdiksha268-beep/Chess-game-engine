// Thin C interface so JavaScript can drive the C++ chess engine.
// It holds one Board and the side to move. The engine itself is unchanged.
#include <string>
#include <vector>
#include "engine/Board.h"
#include "analyzer/MinimaxAI.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define API extern "C" EMSCRIPTEN_KEEPALIVE
#else
#define API extern "C"
#endif

namespace {

Board board;
Color turn = Color::White;
int moveCount = 0;
std::string out;   // text buffer handed back to JavaScript

Color other(Color c) { return c == Color::White ? Color::Black : Color::White; }

char promoLetter(PieceType t) {
    switch (t) {
        case PieceType::Rook:   return 'r';
        case PieceType::Bishop: return 'b';
        case PieceType::Knight: return 'n';
        default:                return 'q';
    }
}

char typeLetter(MoveType t) {
    switch (t) {
        case MoveType::Castle:    return 'c';
        case MoveType::EnPassant: return 'e';
        case MoveType::Promotion: return 'p';
        default:                  return 'n';
    }
}

// "fromRow,fromCol,toRow,toCol,type,promotion"   e.g. "6,4,4,4,n,-"
std::string encode(const Move& m) {
    std::string s = std::to_string(m.from.row) + "," + std::to_string(m.from.col) + "," +
                    std::to_string(m.to.row) + "," + std::to_string(m.to.col) + "," +
                    typeLetter(m.type) + ",";
    s += (m.type == MoveType::Promotion) ? promoLetter(m.promotion) : '-';
    return s;
}

void play(const Move& m) {
    board.makeMove(m);
    moveCount++;
    turn = other(turn);
}

}  // namespace

// Start a new game: White to move.
API void engine_new_game() {
    board.setupStartingPosition();
    turn = Color::White;
    moveCount = 0;
}

// 64 characters, row 0 (rank 8) first. Uppercase = White, lowercase = Black, '.' = empty.
API const char* engine_board() {
    out.clear();
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* p = board.getPiece({r, c});
            out += p ? p->symbol() : '.';
        }
    }
    return out.c_str();
}

// 0 = White to move, 1 = Black to move.
API int engine_turn() { return turn == Color::White ? 0 : 1; }

API int engine_move_count() { return moveCount; }

// 0 = normal, 1 = check, 2 = checkmate, 3 = stalemate (for the side to move).
API int engine_status() {
    if (board.isCheckmate(turn)) return 2;
    if (board.isStalemate(turn)) return 3;
    if (board.isInCheck(turn)) return 1;
    return 0;
}

// All legal moves for the side to move, separated by ';'.
API const char* engine_legal_moves() {
    out.clear();
    for (const Move& m : board.getLegalMoves(turn)) {
        if (!out.empty()) out += ';';
        out += encode(m);
    }
    return out.c_str();
}

// Play a move chosen by the user. promo is 'q', 'r', 'b' or 'n' (0 means queen).
// Returns 1 if the move was legal and has been played, 0 otherwise.
API int engine_play(int fromRow, int fromCol, int toRow, int toCol, int promo) {
    char wanted = promo ? static_cast<char>(promo) : 'q';
    for (const Move& m : board.getLegalMoves(turn)) {
        if (m.from.row != fromRow || m.from.col != fromCol) continue;
        if (m.to.row != toRow || m.to.col != toCol) continue;
        if (m.type == MoveType::Promotion && promoLetter(m.promotion) != wanted) continue;
        Move chosen = m;
        play(chosen);
        return 1;
    }
    return 0;
}

// Let the minimax AI choose and play a move for the side to move.
// Returns the move in the same text format, or "" if the game is over.
API const char* engine_ai_move(int depth) {
    out.clear();
    if (board.getLegalMoves(turn).empty()) return out.c_str();
    MinimaxAI ai(depth);
    Move m = ai.findBestMove(board, turn);
    out = encode(m);
    play(m);
    return out.c_str();
}

// Analyse the position for the side to move WITHOUT playing anything. Returns up to `count`
// best moves, best first, separated by ';'. Each one is
//   "fromRow,fromCol,toRow,toCol,type,promotion,score"
// where score is in centipawns from White's point of view (a forced mate is about +/-100000).
// The board is left exactly as it was.
API const char* engine_analyze(int depth, int count) {
    out.clear();
    MinimaxAI ai(depth);
    const std::vector<ScoredMove> ranked = ai.rankMoves(board, turn);
    for (int i = 0; i < count && i < static_cast<int>(ranked.size()); ++i) {
        const ScoredMove& r = ranked[static_cast<std::size_t>(i)];
        if (i > 0) out += ';';
        out += encode(r.move) + "," + std::to_string(r.score);
    }
    return out.c_str();
}

// Take back the last move. Returns 1 on success, 0 if there is nothing to undo.
API int engine_undo() {
    if (moveCount == 0) return 0;
    board.undoMove();
    moveCount--;
    turn = other(turn);
    return 1;
}

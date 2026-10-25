// Helpers for building positions and inspecting moves in the tests.
// Squares are written the way players write them: "e4", "a1", ...
#pragma once
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>
#include "engine/Bishop.h"
#include "engine/Board.h"
#include "engine/King.h"
#include "engine/Knight.h"
#include "engine/Pawn.h"
#include "engine/Queen.h"
#include "engine/Rook.h"

// "e4" -> Position{row 4, col 4}   (row 0 is rank 8)
inline Position sq(const std::string& name) {
    return Position{8 - (name[1] - '0'), name[0] - 'a'};
}

inline Color other(Color c) { return c == Color::White ? Color::Black : Color::White; }

// Put one piece on the board: 'K' white king, 'q' black queen, ...
inline void put(Board& board, const std::string& square, char letter) {
    const Color color = std::isupper(static_cast<unsigned char>(letter)) ? Color::White : Color::Black;
    std::unique_ptr<Piece> piece;
    switch (std::tolower(static_cast<unsigned char>(letter))) {
        case 'p': piece = std::make_unique<Pawn>(color); break;
        case 'n': piece = std::make_unique<Knight>(color); break;
        case 'b': piece = std::make_unique<Bishop>(color); break;
        case 'r': piece = std::make_unique<Rook>(color); break;
        case 'q': piece = std::make_unique<Queen>(color); break;
        default:  piece = std::make_unique<King>(color); break;
    }
    board.setPiece(sq(square), std::move(piece));
}

// Fill the board from the piece-placement part of a FEN string, e.g.
// "r3k2r/8/8/8/8/8/8/R3K2R"  (rank 8 first, digits are empty squares)
inline void setupFen(Board& board, const std::string& placement) {
    board.clear();
    int row = 0;
    int col = 0;
    for (char ch : placement) {
        if (ch == '/') { row++; col = 0; }
        else if (std::isdigit(static_cast<unsigned char>(ch))) col += ch - '0';
        else {
            put(board, std::string(1, static_cast<char>('a' + col)) + static_cast<char>('0' + (8 - row)), ch);
            col++;
        }
    }
}

// The 64 piece letters of the board, row 0 first ('.' = empty)
inline std::string snapshot(const Board& board) {
    std::string s;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            const Piece* p = board.getPiece({r, c});
            s += p ? p->symbol() : '.';
        }
    return s;
}

// Moves a piece could make by its own rules (before the "own king in check" filter)
inline std::vector<Move> pseudoMoves(const Board& board, const std::string& square) {
    const Piece* p = board.getPiece(sq(square));
    return p ? p->getValidMoves(board, sq(square)) : std::vector<Move>{};
}

inline bool hasMove(const std::vector<Move>& moves, const std::string& from, const std::string& to) {
    return std::any_of(moves.begin(), moves.end(), [&](const Move& m) { return m.from == sq(from) && m.to == sq(to); });
}

inline int countFrom(const std::vector<Move>& moves, const std::string& from) {
    return static_cast<int>(std::count_if(moves.begin(), moves.end(), [&](const Move& m) { return m.from == sq(from); }));
}

inline int countType(const std::vector<Move>& moves, MoveType type) {
    return static_cast<int>(std::count_if(moves.begin(), moves.end(), [&](const Move& m) { return m.type == type; }));
}

// Play a move that is legal for `side`, the way the game does (so castling, en passant
// and promotion are applied properly). promo is 'q', 'r', 'b' or 'n'. Returns false if illegal.
inline bool playLegal(Board& board, Color side, const std::string& from, const std::string& to, char promo = 'q') {
    for (const Move& m : board.getLegalMoves(side)) {
        if (!(m.from == sq(from) && m.to == sq(to))) continue;
        if (m.type == MoveType::Promotion) {
            const PieceType wanted = promo == 'r' ? PieceType::Rook : promo == 'b' ? PieceType::Bishop
                                   : promo == 'n' ? PieceType::Knight : PieceType::Queen;
            if (m.promotion != wanted) continue;
        }
        board.makeMove(m);
        return true;
    }
    return false;
}

inline PieceType typeAt(const Board& board, const std::string& square) {
    return board.getPiece(sq(square))->type();
}

inline bool isEmptyAt(const Board& board, const std::string& square) {
    return board.isEmpty(sq(square));
}

// Counts every position reachable in `depth` moves (used to validate the move generator)
inline long long perft(Board& board, Color side, int depth) {
    const std::vector<Move> moves = board.getLegalMoves(side);
    if (depth == 1) return static_cast<long long>(moves.size());
    long long total = 0;
    for (const Move& m : moves) {
        board.makeMove(m);
        total += perft(board, other(side), depth - 1);
        board.undoMove();
    }
    return total;
}

// The same position with the colours swapped and the board turned upside down
inline std::string mirrorFen(const std::string& placement) {
    std::vector<std::string> ranks;
    std::string current;
    for (char ch : placement) {
        if (ch == '/') { ranks.push_back(current); current.clear(); }
        else current += std::isalpha(static_cast<unsigned char>(ch))
                 ? (std::isupper(static_cast<unsigned char>(ch)) ? static_cast<char>(std::tolower(ch)) : static_cast<char>(std::toupper(ch)))
                 : ch;
    }
    ranks.push_back(current);
    std::reverse(ranks.begin(), ranks.end());
    std::string out;
    for (std::size_t i = 0; i < ranks.size(); ++i) out += (i ? "/" : "") + ranks[i];
    return out;
}

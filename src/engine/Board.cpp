#include "engine/Board.h"
#include <iostream>
#include <utility>
#include "engine/Pawn.h"
#include "engine/Rook.h"
#include "engine/Knight.h"
#include "engine/Bishop.h"
#include "engine/Queen.h"
#include "engine/King.h"
#include <cstdlib>
namespace {
std::unique_ptr<Piece> createPiece(PieceType type, Color color) {
    switch (type) {
        case PieceType::Knight: return std::make_unique<Knight>(color);
        case PieceType::Bishop: return std::make_unique<Bishop>(color);
        case PieceType::Rook:   return std::make_unique<Rook>(color);
        default:                return std::make_unique<Queen>(color);
    }
}
}  // namespace
Board::Board() {}

const Piece* Board::getPiece(Position pos) const {
    if (!isInside(pos.row, pos.col)) {
        return nullptr;
    }
    return grid_[pos.row][pos.col].get();
}

bool Board::isEmpty(Position pos) const {
    return getPiece(pos) == nullptr;
}

void Board::setupStartingPosition() {
    for (auto& rowOfSquares : grid_) {
        for (auto& square : rowOfSquares) {
            square.reset();
        }
    }
    history_.clear();

    const int blackBackRow = 0, blackPawnRow = 1;
    const int whitePawnRow = 6, whiteBackRow = 7;

    for (int col = 0; col < 8; ++col) {
        grid_[blackPawnRow][col] = std::make_unique<Pawn>(Color::Black);
        grid_[whitePawnRow][col] = std::make_unique<Pawn>(Color::White);
    }

    auto placeBackRank = [this](int row, Color color) {
        grid_[row][0] = std::make_unique<Rook>(color);
        grid_[row][1] = std::make_unique<Knight>(color);
        grid_[row][2] = std::make_unique<Bishop>(color);
        grid_[row][3] = std::make_unique<Queen>(color);
        grid_[row][4] = std::make_unique<King>(color);
        grid_[row][5] = std::make_unique<Bishop>(color);
        grid_[row][6] = std::make_unique<Knight>(color);
        grid_[row][7] = std::make_unique<Rook>(color);
    };
    placeBackRank(blackBackRow, Color::Black);
    placeBackRank(whiteBackRow, Color::White);
}

void Board::print() const {
    std::cout << "\n  a b c d e f g h\n";
    for (int row = 0; row < 8; ++row) {
        std::cout << (8 - row) << " ";
        for (int col = 0; col < 8; ++col) {
            const Piece* piece = grid_[row][col].get();
            std::cout << (piece ? piece->symbol() : '.') << ' ';
        }
        std::cout << (8 - row) << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

void Board::makeMove(const Move& move) {
    MoveRecord rec;
    rec.move = move;
    rec.captured = std::move(grid_[move.to.row][move.to.col]);

    Piece* piece = grid_[move.from.row][move.from.col].get();
    rec.movedBefore = piece->hasMoved();

    if (move.type == MoveType::Promotion) {
        Color color = piece->color();
        rec.promotedPawn = std::move(grid_[move.from.row][move.from.col]);   // keep the pawn for undo
        grid_[move.to.row][move.to.col] = createPiece(move.promotion, color);
        grid_[move.to.row][move.to.col]->setMoved(true);
    } 
    else if (move.type == MoveType::Castle) {
        bool kingside = move.to.col > move.from.col;
        rec.castled = true;
        rec.rookFrom = {move.from.row, kingside ? 7 : 0};
        rec.rookTo   = {move.from.row, kingside ? 5 : 3};

        grid_[move.to.row][move.to.col] = std::move(grid_[move.from.row][move.from.col]);
        piece->setMoved(true);

        grid_[rec.rookTo.row][rec.rookTo.col] =
            std::move(grid_[rec.rookFrom.row][rec.rookFrom.col]);
        grid_[rec.rookTo.row][rec.rookTo.col]->setMoved(true);
    }
    else if (move.type == MoveType::EnPassant) {
        rec.enPassant = true;
        rec.capturedPos = {move.from.row, move.to.col};                    // pawn beside us
        rec.captured = std::move(grid_[rec.capturedPos.row][rec.capturedPos.col]);
        grid_[move.to.row][move.to.col] = std::move(grid_[move.from.row][move.from.col]);
        piece->setMoved(true);
    }
    else {
        grid_[move.to.row][move.to.col] = std::move(grid_[move.from.row][move.from.col]);
        piece->setMoved(true);
    }

    history_.push_back(std::move(rec));
}

void Board::undoMove() {
    if (history_.empty()) return;
    MoveRecord& rec = history_.back();
        if (rec.enPassant) {
        grid_[rec.move.from.row][rec.move.from.col] =
            std::move(grid_[rec.move.to.row][rec.move.to.col]);
        grid_[rec.move.from.row][rec.move.from.col]->setMoved(rec.movedBefore);
        grid_[rec.capturedPos.row][rec.capturedPos.col] = std::move(rec.captured);
        history_.pop_back();
        return;     // done: the normal undo code below would put the pawn on the wrong square
    }
    if (rec.castled) {
        grid_[rec.rookFrom.row][rec.rookFrom.col] =
            std::move(grid_[rec.rookTo.row][rec.rookTo.col]);
        grid_[rec.rookFrom.row][rec.rookFrom.col]->setMoved(false);
    }

    if (rec.move.type == MoveType::Promotion) {
        grid_[rec.move.from.row][rec.move.from.col] = std::move(rec.promotedPawn);
        grid_[rec.move.to.row][rec.move.to.col].reset();    // remove the promoted piece
    } 

    else {
        grid_[rec.move.from.row][rec.move.from.col] =
            std::move(grid_[rec.move.to.row][rec.move.to.col]);
    }
    grid_[rec.move.from.row][rec.move.from.col]->setMoved(rec.movedBefore);
    grid_[rec.move.to.row][rec.move.to.col] = std::move(rec.captured);

    history_.pop_back();
}

bool Board::isInCheck(Color color) const {
    // 1. Find this colour's king
    Position kingPos{-1, -1};
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* p = grid_[r][c].get();
            if (p != nullptr && p->color() == color && p->type() == PieceType::King) {
                kingPos = {r, c};
            }
        }
    }
    if (kingPos.row == -1) return false;   // no king on the board

    // 2. Does any enemy piece have a move that lands on the king's square?
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* p = grid_[r][c].get();
            if (p == nullptr || p->color() == color) continue;

            for (const Move& m : p->getValidMoves(*this, {r, c})) {
                if (m.to == kingPos) return true;
            }
        }
    }
    return false;
}

std::vector<Move> Board::getLegalMoves(Color color) {
    // 1. Moves that follow each piece's own rules, plus castling and en passant
    std::vector<Move> candidates;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* p = grid_[r][c].get();
            if (p == nullptr || p->color() != color) continue;
            for (const Move& m : p->getValidMoves(*this, {r, c})) {
                candidates.push_back(m);
            }
        }
    }
    addCastlingMoves(color, candidates);
    addEnPassantMoves(color, candidates);

    // 2. Keep only the moves that don't leave our own king in check
    std::vector<Move> legal;
    for (const Move& m : candidates) {
        makeMove(m);
        bool stillInCheck = isInCheck(color);
        undoMove();
        if (!stillInCheck) legal.push_back(m);
    }
    return legal;
}
bool Board::isCheckmate(Color color) {
    return isInCheck(color) && getLegalMoves(color).empty();
}

bool Board::isStalemate(Color color) {
    return !isInCheck(color) && getLegalMoves(color).empty();
}

void Board::clear() {
    for (auto& rowOfSquares : grid_) {
        for (auto& square : rowOfSquares) {
            square.reset();
        }
    }
    history_.clear();
}

void Board::setPiece(Position pos, std::unique_ptr<Piece> piece) {
    if (isInside(pos.row, pos.col)) {
        grid_[pos.row][pos.col] = std::move(piece);
    }
} 

void Board::addCastlingMoves(Color color, std::vector<Move>& out) {
    int row = (color == Color::White) ? 7 : 0;

    const Piece* king = getPiece({row, 4});
    if (king == nullptr || king->type() != PieceType::King ||
        king->color() != color || king->hasMoved()) return;
    if (isInCheck(color)) return;

    // True if the king can step one square to this column without being in check
    auto kingCanPassThrough = [&](int col) {
        makeMove(Move{{row, 4}, {row, col}});
        bool attacked = isInCheck(color);
        undoMove();
        return !attacked;
    };

    auto unmovedRook = [&](int col) {
        const Piece* r = getPiece({row, col});
        return r != nullptr && r->type() == PieceType::Rook &&
               r->color() == color && !r->hasMoved();
    };

    // Kingside: king e to g, rook h to f
    if (unmovedRook(7) && isEmpty({row, 5}) && isEmpty({row, 6}) &&
        kingCanPassThrough(5)) {
        out.push_back(Move{{row, 4}, {row, 6}, MoveType::Castle});
    }
    // Queenside: king e to c, rook a to d (b must be empty, but may be attacked)
    if (unmovedRook(0) && isEmpty({row, 1}) && isEmpty({row, 2}) &&
        isEmpty({row, 3}) && kingCanPassThrough(3)) {
        out.push_back(Move{{row, 4}, {row, 2}, MoveType::Castle});
    }
}
void Board::addEnPassantMoves(Color color, std::vector<Move>& out) {
    if (history_.empty()) return;
    const Move& last = history_.back().move;

    // The last move must be a pawn moving two squares straight forward
    if (std::abs(last.to.row - last.from.row) != 2 || last.to.col != last.from.col) return;
    const Piece* moved = getPiece(last.to);
    if (moved == nullptr || moved->type() != PieceType::Pawn || moved->color() == color) return;

    int targetRow = (last.from.row + last.to.row) / 2;   // the square the pawn skipped

    // One of our pawns standing next to it can capture it
    for (int dc = -1; dc <= 1; dc += 2) {
        int col = last.to.col + dc;
        if (!isInside(last.to.row, col)) continue;
        const Piece* mine = getPiece({last.to.row, col});
        if (mine != nullptr && mine->type() == PieceType::Pawn && mine->color() == color) {
            out.push_back(Move{{last.to.row, col}, {targetRow, last.to.col}, MoveType::EnPassant});
        }
    }
}
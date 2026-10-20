#include "engine/Board.h"
#include <iostream>
#include <utility>
#include "engine/Pawn.h"
#include "engine/Rook.h"
#include "engine/Knight.h"
#include "engine/Bishop.h"
#include "engine/Queen.h"
#include "engine/King.h"

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
    // Take whatever is on the target square (nullptr if empty) and keep it
    rec.captured = std::move(grid_[move.to.row][move.to.col]);

    Piece* piece = grid_[move.from.row][move.from.col].get();
    rec.movedBefore = piece->hasMoved();

    // Move the piece and mark it as moved
    grid_[move.to.row][move.to.col] = std::move(grid_[move.from.row][move.from.col]);
    piece->setMoved(true);

    history_.push_back(std::move(rec));
}

void Board::undoMove() {
    if (history_.empty()) return;
    MoveRecord& rec = history_.back();

    // Put the piece back where it came from and restore its moved flag
    grid_[rec.move.from.row][rec.move.from.col] =
        std::move(grid_[rec.move.to.row][rec.move.to.col]);
    grid_[rec.move.from.row][rec.move.from.col]->setMoved(rec.movedBefore);

    // Put back the captured piece (or leave the square empty)
    grid_[rec.move.to.row][rec.move.to.col] = std::move(rec.captured);

    history_.pop_back();
}
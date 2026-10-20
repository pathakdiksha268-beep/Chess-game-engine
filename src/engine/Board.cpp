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
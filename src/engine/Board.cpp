#include "engine/Board.h"
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
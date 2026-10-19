#pragma once
#include "Piece.h"

class SlidingPiece : public Piece {
public:
    SlidingPiece(Color color, PieceType type) : Piece(color, type) {}
    // Implemented once here, shared by Rook, Bishop and Queen
    std::vector<Move> getValidMoves(const Board& board, Position from) const override;
protected:
    // Each subclass supplies its own directions as {row step, column step}
    virtual std::vector<Position> directions() const = 0;
};
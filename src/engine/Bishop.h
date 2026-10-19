#pragma once
#include "SlidingPiece.h"

class Bishop : public SlidingPiece {
public:
    explicit Bishop(Color color) : SlidingPiece(color, PieceType::Bishop) {}

    char symbol() const override { return color() == Color::White ? 'B' : 'b'; }
    int value() const override { return 330; }

protected:
    std::vector<Position> directions() const override {
        return {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};
    }
};
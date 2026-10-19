#pragma once
#include "SlidingPiece.h"

class Rook : public SlidingPiece {
    public:
        explicit Rook(Color color) : SlidingPiece(color,PieceType::Rook){}
        char symbol() const override {
            return color() == Color::White ? 'R' : 'r';
        }
        int value() const override {
            return 500;
        }
    protected:
        std::vector<Position> directions() const override {
            return {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        }
};
#pragma once
#include "SlidingPiece.h"

class Queen : public SlidingPiece {
    public:
        explicit Queen(Color color) : SlidingPiece(color,PieceType::Queen){}
        char symbol() const override {
            return color() == Color::White ? 'Q' : 'q';
        }
        int value() const override {
            return 900;
        }
    protected:
        std::vector<Position> directions() const override {
            return {{-1, 0}, {1, 0}, {0, -1}, {0, 1},{-1,-1},{-1,1},{1,1},{1,-1}};
        }
};
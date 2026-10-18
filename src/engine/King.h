#pragma once
#include "Piece.h"
class King : public Piece{
    public:
        explicit King(Color color) : Piece(color,PieceType::King) {}
        std::vector<Move>getValidMoves(const Board& board,Position from) const override;
        char symbol() const override { 
            return color() == Color::White ? 'K':'k';
        }
        int value() const override {
            return 20000;
        }
};

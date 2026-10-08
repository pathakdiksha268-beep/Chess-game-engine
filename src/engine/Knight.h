#pragma once
#include "Piece.h"
class Knight : public Piece{
    public:
        explicit Knight(Color color) : Piece(color,PieceType::Knight){}
        std::vector<Move> getValidMoves(const Board& board,Position from) const override;
        char symbol() const override {
            return color() == Color::White ? 'N' : 'n';//N -> White Knight || n -> Black Night
        }
        int value() const override{
            return 320;//used by evaluator to determine the piecetype
        }  
};
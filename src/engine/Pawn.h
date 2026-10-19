#pragma once
#include "Piece.h"

class Pawn : public Piece{
public:
    explicit Pawn(Color color) : Piece(color, PieceType::Pawn) {}
    std::vector<Move> getValidMoves(const Board& board, Position from) const override;
    char symbol() const override { 
        return color() == Color::White ? 'P' : 'p'; 
    }
    int value() const override { 
        return 100; 
    }
};
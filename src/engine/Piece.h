#pragma once
#include <vector>
#include "Types.h"
#include "Move.h"

class Board;  // forward declaration avoids a circular include

class Piece{
public:
    Piece(Color color, PieceType type) : color_(color), type_(type) {}
    virtual ~Piece() = default;

    virtual std::vector<Move> getValidMoves(const Board& board, Position from) const = 0;
    virtual char symbol() const = 0;  
    virtual int value() const = 0;     // material value for the evaluator

    Color color() const{
        return color_;
    }
    PieceType type() const{
        return type_;
    }
    bool hasMoved() const{ 
        return hasMoved_; 
    }
    void setMoved(bool moved){ 
        hasMoved_ = moved; 
    }
private:
    Color color_;
    PieceType type_;
    bool hasMoved_ = false;
};
// Base class created to avoid duplicating common piece information in every derived piece class.
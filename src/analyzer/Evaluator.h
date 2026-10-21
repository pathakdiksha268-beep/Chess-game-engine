#pragma once
#include "engine/Board.h"

class Evaluator{
    public:
    // Positive score = White is ahead, negative = Black is ahead
    int evaluate(const Board& board) const;
    private:
    // Bonus or penalty for this piece standing on this square
    int pieceSquareValue(const Piece& piece, Position pos) const;
};
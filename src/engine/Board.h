#pragma once
#include <array>
#include <memory>
#include <vector>
#include "Types.h"
#include "Move.h"
#include "Piece.h"

class Board {
public:
    Board();
    void setupStartingPosition();
    void print() const;

    static bool isInside(int row, int col) {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
    const Piece* getPiece(Position pos) const;
    bool isEmpty(Position pos) const;

    void makeMove(const Move& move);
    void undoMove();

    bool isInCheck(Color color) const;
    std::vector<Move> getLegalMoves(Color color);  // makes and undoes moves, so not const

private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid_;
};
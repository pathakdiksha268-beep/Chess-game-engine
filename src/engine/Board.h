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

    // Declared but not written yet (next tasks)
    bool isInCheck(Color color) const;
    std::vector<Move> getLegalMoves(Color color);

private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid_;

    // Everything undoMove needs to reverse one move
    struct MoveRecord {
        Move move;
        std::unique_ptr<Piece> captured;   // piece that was taken, if any
        bool movedBefore = false;          // had the moved piece moved earlier?
    };
    std::vector<MoveRecord> history_;
};
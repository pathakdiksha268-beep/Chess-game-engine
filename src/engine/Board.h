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
    std::vector<Move> getLegalMoves(Color color);
    bool isCheckmate(Color color);
    bool isStalemate(Color color);

    // Helpers for building test positions
    void clear();
    void setPiece(Position pos, std::unique_ptr<Piece> piece);

private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid_;

    // Everything undoMove needs to reverse one move
        struct MoveRecord {
        Move move;
        std::unique_ptr<Piece> captured;      // piece that was taken, if any
        std::unique_ptr<Piece> promotedPawn;  // the pawn that was replaced, if promotion
        bool movedBefore = false;
        bool castled = false;          //was this a castling move?
        Position rookFrom{0, 0};       //where the rook started
        Position rookTo{0, 0};         // where the rook went
    };
    std::vector<MoveRecord> history_;
    void addCastlingMoves(Color color, std::vector<Move>& out);
};
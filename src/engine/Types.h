#pragma once

enum class Color { White, Black };
enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King };

struct Position {
    int row = 0;
    int col = 0;
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};
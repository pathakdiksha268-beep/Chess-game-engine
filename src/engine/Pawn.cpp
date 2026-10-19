#include "Pawn.h"
#include "Board.h"
using namespace std;

vector<Move> Pawn::getValidMoves(const Board& board, Position from) const {
    vector<Move> moves;
    int dir = (color() == Color::White) ? -1 : 1;  
    int startRow = (color() == Color::White) ? 6 : 1;
    // 1 and 2: forward moves (the target squares must be empty)
    int oneRow = from.row + dir;
    if (Board::isInside(oneRow, from.col) && board.isEmpty({oneRow, from.col})) {
        moves.push_back(Move{from, {oneRow, from.col}});
        // two steps only from the start row, and only if one step was free
        int twoRow = from.row + 2 * dir;
        if (from.row == startRow && board.isEmpty({twoRow, from.col})) {
            moves.push_back(Move{from, {twoRow, from.col}});
        }
    }
    // 3: diagonal captures (the target square must hold an enemy)
    for (int dc = -1; dc <= 1; dc += 2) {
        int r = from.row + dir;
        int c = from.col + dc;
        if (!Board::isInside(r, c)) continue;
        const Piece* target = board.getPiece({r, c});
        if (target != nullptr && target->color() != color()) {
            moves.push_back(Move{from, {r, c}});
        }
    }
    return moves;
}
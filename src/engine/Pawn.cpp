#include "Pawn.h"
#include "Board.h"
using namespace std;

vector<Move> Pawn::getValidMoves(const Board& board, Position from) const {
    vector<Move> moves;
    int dir = (color() == Color::White) ? -1 : 1;        // White moves up (row decreases)
    int startRow = (color() == Color::White) ? 6 : 1;
    int lastRow = (color() == Color::White) ? 0 : 7;

    // Adds a normal move, or four promotion moves if the pawn reaches the last row
    auto addMove = [&](Position to) {
        if (to.row == lastRow) {
            for (PieceType t : {PieceType::Queen, PieceType::Rook,
                                PieceType::Bishop, PieceType::Knight}) {
                Move m{from, to, MoveType::Promotion};
                m.promotion = t;
                moves.push_back(m);
            }
        } else {
            moves.push_back(Move{from, to});
        }
    };

    // Forward moves (the target squares must be empty)
    int oneRow = from.row + dir;
    if (Board::isInside(oneRow, from.col) && board.isEmpty({oneRow, from.col})) {
        addMove({oneRow, from.col});

        int twoRow = from.row + 2 * dir;
        if (from.row == startRow && board.isEmpty({twoRow, from.col})) {
            moves.push_back(Move{from, {twoRow, from.col}});
        }
    }

    // Diagonal captures (the target square must hold an enemy)
    for (int dc = -1; dc <= 1; dc += 2) {
        int r = from.row + dir;
        int c = from.col + dc;
        if (!Board::isInside(r, c)) continue;
        const Piece* target = board.getPiece({r, c});
        if (target != nullptr && target->color() != color()) {
            addMove({r, c});
        }
    }

    return moves;
}
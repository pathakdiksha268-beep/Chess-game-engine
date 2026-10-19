#include "SlidingPiece.h"
#include "Board.h"
using namespace std;

vector<Move> SlidingPiece::getValidMoves(const Board& board, Position from) const {
    vector<Move> moves;
    for (Position d : directions()) {
        int r = from.row + d.row;
        int c = from.col + d.col;              // fix 1: from.col
        while (Board::isInside(r, c)) {
            const Piece* target = board.getPiece({r, c});
            if (target == nullptr) {
                moves.push_back(Move{from, {r, c}});   // fix 2: add the move
                r += d.row;
                c += d.col;
            } else if (target->color() != color()) {
                moves.push_back(Move{from, {r, c}});   // fix 2: capture is a move
                break;                                  // fix 3: no stepping before break
            } else {
                break;                                  // own piece blocks, no move
            }
        }
    }
    return moves;
}
#include "King.h"
#include "Board.h"
using namespace std;

vector<Move>King::getValidMoves(const Board& board,Position from) const{
    vector<Move>moves;
    //implementing king moves on row and column
    const int dirRow[] = {-1,-1,-1,0,0,1,1,1};
    const int dirCol[] = {-1,0,1,-1,1,-1,0,1};
    for(int i=0;i<8;++i){
        int r = from.row + dirRow[i];
        int c = from.col + dirCol[i];
        if(!Board::isInside(r,c)) continue;
        const Piece* target = board.getPiece({r,c});
        if(target == nullptr || target->color() != color()){
            moves.push_back(Move{from,{r,c}});
        }
    }
}

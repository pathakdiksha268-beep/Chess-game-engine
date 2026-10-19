#include "Knight.h"
#include "Board.h"
using namespace std;

vector<Move> Knight::getValidMoves(const Board& board,Position from) const{
    vector<Move>moves;
    //Implementation of knight moves on row and column 
    const int dirRow[] = {-2,-2,-1,-1,1,1,2,2}; 
    const int dirCol[] = {-1,1,-2,2,-2,2,-1,1};
    for(int i=0;i<8;++i){
        int r = from.row + dirRow[i];
        int c = from.col + dirCol[i];
        if(!Board::isInside(r,c)) continue;
        const Piece* target = board.getPiece({r,c});
        if(target == nullptr || target->color() != color()){
            moves.push_back(Move{from,{r,c}});
        }
    }
    return moves;
}
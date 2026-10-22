#pragma once
#include <string>
#include "Board.h"
#include "analyzer/MinimaxAI.h"

class Game {
public:
    Game() = default;
    void run();   // the main loop

private:
    Board board_;
    Color turn_ = Color::White;
    int moveCount_ = 0;

    bool vsAI_ = false;                 // true in Human vs AI mode
    Color aiColor_ = Color::Black;      // the AI plays Black, the human plays White
    MinimaxAI ai_;                      // default search depth is 2

    bool chooseMode();                  // asks for 1 or 2; returns false if input was closed
    bool parseMove(const std::string& input, Move& out) const;  // "e2 e4" into a Move
    void switchTurn();
};
   #pragma once
   #include <string>
   #include "Board.h"

   class Game {
   public:
       Game() = default;
       void run();   // the main loop

   private:
       Board board_;
       Color turn_ = Color::White;
       int moveCount_ = 0;

       bool parseMove(const std::string& input, Move& out) const;  // "e2 e4" into a Move
       void switchTurn();
   };
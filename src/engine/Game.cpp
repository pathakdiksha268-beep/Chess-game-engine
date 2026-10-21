#include "Game.h"
#include <iostream>
#include <cctype>

void Game::switchTurn() {
    turn_ = (turn_ == Color::White) ? Color::Black : Color::White;
}

bool Game::parseMove(const std::string& input, Move& out) const {
    // Remove spaces and make lowercase: "E2 e4" -> "e2e4"
    std::string s;
    for (char c : input) {
        if (!std::isspace(static_cast<unsigned char>(c)))
            s += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    if (s.size() != 4 && s.size() != 5) return false;

    // Letters must be a-h, digits must be 1-8
    if (s[0] < 'a' || s[0] > 'h' || s[2] < 'a' || s[2] > 'h') return false;
    if (s[1] < '1' || s[1] > '8' || s[3] < '1' || s[3] > '8') return false;

    out.from.col = s[0] - 'a';
    out.from.row = 8 - (s[1] - '0');   // rank 8 is printed at the top, so e2 is row 6
    out.to.col   = s[2] - 'a';
    out.to.row   = 8 - (s[3] - '0');
    out.type = MoveType::Normal;
    out.promotion = PieceType::Queen;

    // Optional 5th character for promotion: q, r, b or n
    if (s.size() == 5) {
        switch (s[4]) {
            case 'q': out.promotion = PieceType::Queen;  break;
            case 'r': out.promotion = PieceType::Rook;   break;
            case 'b': out.promotion = PieceType::Bishop; break;
            case 'n': out.promotion = PieceType::Knight; break;
            default: return false;
        }
    }
    return true;
}

void Game::run() {
    board_.setupStartingPosition();
    std::cout << "Type moves like 'e2 e4'. Commands: undo, quit\n";

    while (true) {
        board_.print();
        std::cout << (turn_ == Color::White ? "White" : "Black") << " to move\n";

        if (board_.isCheckmate(turn_)) {
            std::cout << "Checkmate! "
                      << (turn_ == Color::White ? "Black" : "White") << " wins.\n";
            break;
        }
        if (board_.isStalemate(turn_)) {
            std::cout << "Stalemate. It's a draw.\n";
            break;
        }
        if (board_.isInCheck(turn_)) {
            std::cout << "Check!\n";
        }

        std::cout << "> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;   // input closed

        // Make a trimmed, lowercase copy to check for commands
        std::string cmd;
        for (char c : line) {
            if (!std::isspace(static_cast<unsigned char>(c)))
                cmd += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        if (cmd == "quit") break;

        if (cmd == "undo") {
            if (moveCount_ == 0) {
                std::cout << "Nothing to undo.\n";
            } else {
                board_.undoMove();
                moveCount_--;
                switchTurn();
            }
            continue;
        }

        Move typed;
        if (!parseMove(line, typed)) {
            std::cout << "Invalid input. Use a format like 'e2 e4'.\n";
            continue;
        }

        // Find the matching move in the list of legal moves
        Move chosen;
        bool found = false;
        for (const Move& m : board_.getLegalMoves(turn_)) {
            if (m.from == typed.from && m.to == typed.to) {
                chosen = m;
                found = true;
                break;
            }
        }
        if (!found) {
            std::cout << "Illegal move. Try again.\n";
            continue;
        }

        if (chosen.type == MoveType::Promotion) {
            chosen.promotion = typed.promotion;   // piece the player chose (default queen)
        }

        board_.makeMove(chosen);
        moveCount_++;
        switchTurn();
    }
}
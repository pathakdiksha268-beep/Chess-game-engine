# Chess Engine with Minimax AI

A complete console-based chess game in C++ with an AI opponent powered by minimax search.

## Problem Statement

Chess has many piece types, each with its own movement rules, plus special moves and end conditions. Implementing it is a strong exercise in object-oriented design: every piece behaves differently, yet the board must treat them all the same way. This project builds a console chess game that:

- models pieces with a deep inheritance hierarchy and virtual functions
- generates and validates moves through a polymorphic `getValidMoves()`
- plays against the user using minimax search with alpha-beta pruning
- scores positions using a matrix-based evaluation (material plus piece-square tables)

## Features

- Console board display with algebraic input (e.g. `e2 e4`)
- Legal move validation, check, checkmate, and stalemate detection
- Castling (see tests), en passant and pawn promotion (in progress)
- Human vs Human and Human vs AI modes
- AI with adjustable search depth
- Move history and undo

## OOP Design

**Inheritance hierarchy**

```
Piece (abstract)
 |-- Pawn
 |-- Knight
 |-- King
 |-- SlidingPiece (abstract)
      |-- Rook
      |-- Bishop
      |-- Queen
```

- `Piece` declares `virtual std::vector<Move> getValidMoves(const Board&) const = 0;`
- Each derived class overrides it with its own movement rules.
- `SlidingPiece` holds the shared direction-scanning logic for Rook, Bishop, and Queen.
- `Board` stores pieces as `Piece*` and works through the base class (polymorphism).


**Other classes:** `Board`, `Move`, `Position`, `Game` (holds a `vsAI_` flag for Human vs AI), `Evaluator`, `MinimaxAI`.

## AI and Evaluation

- **Search:** minimax with alpha-beta pruning, configurable depth.
- **Evaluation matrix:** score = material value + piece-square table value. Each piece type has an 8x8 matrix rewarding good squares (e.g. central knights, advanced pawns), mirrored for the other colour.

## Project Structure

```
src/engine/   Piece classes, Board, Move, Game (game loop)
src/analyzer/ Evaluator and MinimaxAI
src/main.cpp  Starts the game
tests/        Move generation and rule tests
docs/       Class diagrams and report
```

## Build and Run

```bash
g++ -std=c++17 -o chess $(find src -name '*.cpp')
./chess
```

## How to Play

Choose a mode at the start: 1) Human vs Human, 2) Human vs AI (you play White).
Enter moves as the starting and ending square, e.g. `e2 e4`.
Commands: `undo` (takes back two moves in AI mode) and `quit`.
The program prints "Check!", "Checkmate!" or "Stalemate" when they happen.


## Team

| Name | GitHub | Area |
|---|---|---|
| _Name_ | pathakdiksha268-beep | _e.g. pieces and board_ |
| Ridhi Sihag | ridhisihag22 | Game loop, input handling, Human vs AI mode, class diagrams |
## Git Workflow

- `main` always contains working code; never push directly to it.
- One branch per task (e.g. `feature/knight-moves`), merged through a pull request.
- Small commits with clear messages; run `git pull` before starting new work.

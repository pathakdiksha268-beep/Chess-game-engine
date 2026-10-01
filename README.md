# Chess-game-engine
# Chess Game Engine

An object-oriented chess engine in C++ with a GUI and a built-in move analyzer.

## Problem Statement

Chess has many piece types, special moves, and end conditions, and beginner implementations often mix game logic with the UI and give players no insight into why a move is good or bad. This project builds a complete chess system that:

- models the board, pieces, and moves using clean OOP (inheritance, polymorphism, abstraction, encapsulation)
- enforces the full rules: check, checkmate, stalemate, castling, en passant, promotion
- provides a GUI for interactive play, kept separate from the engine
- includes an analyzer that evaluates positions and suggests the best move

## Features

- Legal move generation, move history, and undo
- Clickable board with highlighted legal moves and game status messages
- AI opponent using minimax with alpha-beta pruning
- Analyzer: evaluation bar, best move suggestion, optional game review and PGN support

## OOP Design

- **Abstraction / inheritance / polymorphism:** abstract `Piece` with `Pawn`, `Knight`, `Bishop`, `Rook`, `Queen`, `King`
- **Strategy pattern:** `Analyzer` interface with swappable implementations
- **MVC:** engine (model), GUI (view), click handling (controller)

## Structure

```
src/engine/    pieces, board, moves, rules
src/analyzer/  analyzer interface and implementations
src/gui/       view and controller
tests/
docs/
```

## Tech Stack

C++17 | GUI: _TBD_ | Build: _TBD_ | Git and GitHub

## Getting Started

```bash
git clone https://github.com/pathakdiksha268-beep/Chess-game-engine.git
cd Chess-game-engine
```

## Team

| Name | GitHub |
|---|---|
| _Name_ | pathakdiksha268-beep |
| _Name_ | ridhisihag22 |


# Chess Game Engine — Project Planning

## 1. Project Goal

The goal of this project is to develop a complete chess game engine in C++ using Object-Oriented Programming principles.

The project will support:

* Standard chess rules and legal move validation
* Different chess pieces using inheritance and polymorphism
* Check, checkmate, and stalemate detection
* Special moves such as castling, en passant, and pawn promotion
* Human vs Human gameplay
* Human vs AI gameplay
* AI decision-making using Minimax with Alpha-Beta pruning
* Board evaluation using material values and piece-square tables
* Move history and undo functionality
* A graphical user interface as a future extension

---

## 2. Development Phases

### Phase 1 — Project Setup

* [x] Create GitHub repository
* [x] Set up C++17 project structure
* [x] Create `src/`, `tests/`, and `docs/` directories
* [x] Establish Git branching and pull-request workflow
* [x] Define basic class structure

### Phase 2 — Chess Board and Piece System

* [x] Create abstract `Piece` base class
* [ ] Implement `Pawn`
* [ ] Implement `Knight`
* [ ] Implement `Bishop`
* [ ] Implement `Rook`
* [ ] Implement `Queen`
* [ ] Implement `King`
* [ ] Create `SlidingPiece` abstraction for Bishop, Rook, and Queen
* [ ] Implement `Board`
* [ ] Implement `Move`

### Phase 3 — Move Generation and Validation

* [ ] Implement movement rules for all pieces
* [ ] Generate valid moves
* [ ] Validate moves against board state
* [ ] Implement capture logic
* [ ] Prevent moves that leave the player's king in check
* [ ] Implement check detection
* [ ] Implement checkmate detection
* [ ] Implement stalemate detection

### Phase 4 — Special Chess Rules

* [ ] Implement castling
* [ ] Implement en passant
* [ ] Implement pawn promotion
* [ ] Handle special move conditions and restrictions

### Phase 5 — Game Management

* [ ] Implement `Game` class
* [ ] Implement player management
* [ ] Implement Human vs Human mode
* [ ] Implement Human vs AI mode
* [ ] Implement move history
* [ ] Implement undo functionality
* [ ] Implement game-end conditions

### Phase 6 — AI

* [ ] Create `Evaluator` class
* [ ] Implement material-based evaluation
* [ ] Implement piece-square tables
* [ ] Implement position evaluation
* [ ] Implement Minimax search
* [ ] Add Alpha-Beta pruning
* [ ] Add configurable search depth
* [ ] Improve AI playing strength
* [ ] Experiment with additional evaluation heuristics

### Phase 7 — Testing

* [ ] Test individual piece movement
* [ ] Test captures
* [ ] Test check detection
* [ ] Test checkmate
* [ ] Test stalemate
* [ ] Test castling
* [ ] Test en passant
* [ ] Test pawn promotion
* [ ] Test AI move generation
* [ ] Add more edge-case tests
* [ ] Perform complete game simulations

### Phase 8 — Documentation

* [ ] Create class diagram
* [ ] Document project architecture
* [ ] Document OOP concepts used
* [ ] Document AI implementation
* [ ] Add detailed design documentation
* [ ] Add testing documentation
* [ ] Prepare project presentation
* [ ] Prepare final project report

### Phase 9 — GUI

The console version will serve as the core chess engine. A graphical interface will be added on top of the existing engine without changing the core game logic.

Planned GUI features:

* [ ] Graphical chess board
* [ ] Chess piece graphics
* [ ] Mouse-based piece selection
* [ ] Legal move highlighting
* [ ] Capturing pieces through the interface
* [ ] Check/checkmate indicators
* [ ] Pawn promotion interface
* [ ] Castling and en passant through GUI
* [ ] Human vs Human GUI mode
* [ ] Human vs AI GUI mode
* [ ] Undo button
* [ ] Restart/new game option
* [ ] Game status display

---

## 3. OOP Objectives

The project is specifically designed to demonstrate Object-Oriented Programming concepts.

### Encapsulation

Classes such as `Board`, `Game`, `Move`, and the different piece classes encapsulate their respective data and operations.

### Inheritance

Chess pieces inherit from the abstract `Piece` class.

```text
Piece
├── Pawn
├── Knight
├── King
└── SlidingPiece
    ├── Rook
    ├── Bishop
    └── Queen
```

### Polymorphism

The board can work with different chess pieces through the common `Piece` interface.

Each derived piece provides its own implementation of:

```cpp
getValidMoves()
```

### Abstraction

Common behaviour is separated into abstract classes such as `Piece` and `SlidingPiece`, allowing shared functionality to be implemented once while keeping piece-specific behaviour separate.

### Composition

Classes such as `Game`, `Board`, `Player`, and `Move` work together to represent the overall chess system.

---
## 4. Git Workflow

All development should follow a feature-based workflow.

### Branches

Each major task should have its own branch.

Examples:

```text
feature/pawn-moves
feature/check-detection
feature/castling
feature/minimax
feature/gui
feature/testing
```

### Development Process

```text
Create issue/task
      ↓
Create feature branch
      ↓
Implement feature
      ↓
Test locally
      ↓
Commit changes
      ↓
Push branch
      ↓
Create Pull Request
      ↓
Review
      ↓
Merge into main
```

The `main` branch should always contain working code.

---

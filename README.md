# Chess Engine with Minimax AI

A complete chess game written in C++ with an AI opponent powered by minimax search. It has a console version and a browser version, and both run the same C++ engine.

## Problem Statement

Chess has many piece types, each with its own movement rules, plus special moves and end conditions. Implementing it is a strong exercise in object-oriented design: every piece behaves differently, yet the board must treat them all the same way. This project builds a chess game that:

- models pieces with a deep inheritance hierarchy and virtual functions
- generates and validates moves through a polymorphic `getValidMoves()`
- plays against the user using minimax search with alpha-beta pruning
- scores positions using a matrix-based evaluation (material plus piece-square tables)

## Features

**Rules**
- Legal move validation, check, checkmate and stalemate detection
- Castling, en passant and pawn promotion (with a choice of piece)
- Move history and undo

**Playing**
- Human vs Human and Human vs AI, in the console or in the browser
- Three difficulty levels (the AI looks 1, 2 or 3 moves ahead)

**Browser version**
- Home screen, layout screen (five board themes, coordinates, legal-move highlights) and game screen
- Move list in standard chess notation, captured pieces, material lead, flip board, promotion picker
- **Hint and analysis** (works against the computer and with two players): the engine shows its best move for the side to move as an arrow, an evaluation bar, and its top three moves with scores. One person can play both sides of a game and ask for help for whoever is to move.

**Quality**
- 111 automated tests, including perft counts checked against published numbers
- Builds with CMake on Windows, macOS and Linux

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

- `Piece` declares `virtual std::vector<Move> getValidMoves(const Board& board, Position from) const = 0;`
- Each derived class overrides it with its own movement rules.
- `SlidingPiece` holds the shared direction-scanning logic for Rook, Bishop and Queen; each subclass only supplies its directions.
- `Board` owns its pieces through `std::unique_ptr<Piece>` and works through the base class (polymorphism).
- A `Move` carries a type (normal, castle, en passant, promotion), so one struct describes every kind of move.

**Other classes:** `Board`, `Move`, `Position`, `Game` (console loop), `Evaluator`, `MinimaxAI`.

Class diagrams are in `docs/`.

## AI and Evaluation

- **Search:** minimax with alpha-beta pruning at a configurable depth. Checkmate is scored highly, and a quicker mate scores better.
- **Evaluation matrices:** score = material value + piece-square table value. Each piece type has an 8x8 matrix rewarding good squares (central knights, advanced pawns, a tucked-away king), flipped for the other colour. Positive scores favour White.
- **Analysis:** `MinimaxAI::rankMoves` scores every legal move and returns them best first. The hint, the arrows, the top three moves and the evaluation bar are all built on it. The engine looks 2 to 3 moves ahead, so its advice is sensible but much weaker than a dedicated engine such as Stockfish.

## Project Structure

```
src/engine/    Piece classes, Board, Move, Game (console loop)
src/analyzer/  Evaluator and MinimaxAI
src/web/       wasm_api.cpp: the small C interface the browser page calls
src/main.cpp   Console game
web/           Browser version (index.html, style.css, app.js, chess.js)
tests/         Automated tests
docs/          Class diagrams and report
CMakeLists.txt, build_tests.sh, build_web.sh
```

## Build and Run

Needs a C++17 compiler and CMake 3.16 or newer.

```bash
cmake -S . -B build
cmake --build build
```

| Program | Command |
|---|---|
| Console game | `./build/chess` |
| Tests | `./build/run_tests` |

On Windows with Visual Studio, build with `cmake --build build --config Release` and run `build\Release\chess.exe`.

Without CMake on macOS or Linux:

```bash
g++ -std=c++17 -O2 -Isrc -o chess src/main.cpp src/engine/*.cpp src/analyzer/*.cpp
./chess
```

### Browser version

Open `web/index.html` in a browser. No installation, no server and no build step are needed: the C++ engine, compiled to WebAssembly, is inside `web/chess.js`. It was tested in Firefox; as it uses standard WebAssembly, Chrome, Edge and Safari are expected to work as well.

To rebuild `web/chess.js` from the C++ source (needs [Emscripten](https://emscripten.org), e.g. `brew install emscripten`):

```bash
./build_web.sh
```

## How to Play

**Console.** Choose a mode, then enter moves as the starting and ending square, e.g. `e2 e4`. Add a letter to choose a promotion piece (`a7 a8 n`; default is queen). Commands: `undo` and `quit`. The program prints "Check!", "Checkmate!" or "Stalemate" when they happen.

**Browser.** Pick a mode, choose a layout and press Start. Click a piece to see its legal moves, then click a highlighted square. Buttons: **Hint** (key `H`), Undo, Flip board, New game, Menu. In the move list, `+` means check and `#` means checkmate. On the evaluation display, a positive number is better for White, and `M3` means mate in 3.

## Testing

```bash
./build_tests.sh          # macOS / Linux: builds and runs everything
./build_tests.sh Castling # run only the tests whose name contains "Castling"
```

| Area | What is checked |
|---|---|
| Pieces | Move counts and blocking rules for every piece |
| Board | Make and undo restore the position exactly |
| Check, checkmate, stalemate | Pins, fool's mate, scholar's mate, back-rank mate, stalemate |
| Castling, en passant, promotion | Legal and illegal cases, both colours, undo |
| Evaluator and AI | Start position scores 0, mirrored positions give opposite scores, mate in one is found, a free queen is taken |
| Analysis | Every legal move is ranked once, best first, and the board is left untouched |
| Perft | Counts every reachable position (up to 197,281 deep) from six standard positions and compares them with the official numbers |

## Limitations

- Draws by repetition, the fifty-move rule and insufficient material are not detected; only stalemate is.
- In the browser, the search runs on the page itself, so the Advanced level (and a hint at that level) can pause for a second or two.
- The analysis shown is from this project's own minimax engine, not a professional one.

## Team

| Name | GitHub | Area |
|---|---|---|
| Diksha Pathak | pathakdiksha268-beep | Piece hierarchy, move generation, undo, check/checkmate/stalemate, castling, en passant, promotion, evaluation matrices, minimax AI with alpha-beta pruning and move ranking, browser version (WebAssembly), hint and analysis, test suite, CMake build |
| Ridhi Sihag | ridhisihag22 | Game loop, input handling, Human vs AI mode, class diagrams |

## Git Workflow

- `main` always contains working code; never push directly to it.
- One branch per task (e.g. `feature/knight-moves`), merged through a pull request.
- Small commits with clear messages; run `git pull` before starting new work.

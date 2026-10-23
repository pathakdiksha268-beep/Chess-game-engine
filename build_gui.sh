#!/bin/sh
# Builds the SFML graphical version of the chess game (macOS with Homebrew: brew install sfml)
# Output: build/chess_gui
set -e
cd "$(dirname "$0")"
mkdir -p build
SFML=$(brew --prefix sfml)

g++ -std=c++17 -O2 -Wall -Wextra -Isrc -I"$SFML/include" \
  src/gui/*.cpp src/engine/*.cpp src/analyzer/*.cpp \
  -L"$SFML/lib" -lsfml-graphics -lsfml-window -lsfml-system \
  -o build/chess_gui

echo "Built build/chess_gui   (run it with: ./build/chess_gui)"

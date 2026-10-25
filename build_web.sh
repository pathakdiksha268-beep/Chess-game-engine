#!/bin/sh
# Builds the C++ chess engine for the browser (WebAssembly).
# Needs Emscripten:  brew install emscripten   (macOS)
# Output: web/chess.js  (the engine and the WebAssembly are in this one file)
set -e
cd "$(dirname "$0")"

OPT="${OPT:--O2}"

# Game.cpp is the console game loop, which the web page does not need.
ENGINE_FILES=$(ls src/engine/*.cpp | grep -v 'Game.cpp')

em++ -std=c++17 "$OPT" -Wall -Wextra -Isrc \
  src/web/wasm_api.cpp $ENGINE_FILES src/analyzer/*.cpp \
  -o web/chess.js \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=createChessModule \
  -s EXPORTED_RUNTIME_METHODS=cwrap \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s SINGLE_FILE=1

echo "Built web/chess.js"
echo "Open web/index.html in any browser. No server is needed."

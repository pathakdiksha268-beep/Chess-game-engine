#!/bin/sh
# Builds and runs the test suite.   Usage:  ./build_tests.sh [name-filter]
#   ./build_tests.sh              run every test
#   ./build_tests.sh Castling     run only tests whose name contains "Castling"
set -e
cd "$(dirname "$0")"
mkdir -p build

# the engine without the console game loop (Game.cpp has no tests) plus the AI
ENGINE_FILES=$(ls src/engine/*.cpp | grep -v 'Game.cpp')

g++ -std=c++17 -O2 -Wall -Wextra -Isrc -Itests \
  tests/run_tests.cpp tests/test_*.cpp $ENGINE_FILES src/analyzer/*.cpp \
  -o build/run_tests

./build/run_tests "$@"

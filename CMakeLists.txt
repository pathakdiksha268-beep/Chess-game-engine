# Chess Engine with Minimax AI
#
# Builds on Windows, macOS and Linux:
#   cmake -S . -B build
#   cmake --build build
#
# Programs produced (in the build folder):
#   chess       the console game                (needs only a C++17 compiler)
#   run_tests   the automated tests             (needs only a C++17 compiler)
#
# The graphical version is the web page in web/ (open web/index.html in any browser).
# It runs the same engine compiled to WebAssembly; see build_web.sh.
cmake_minimum_required(VERSION 3.16)
project(ChessEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimised by default: the AI is much faster than in a debug build
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
  add_compile_options(/utf-8)
endif()

# ---- the engine: pieces, board, rules, game loop and the AI (no libraries needed) ----
file(GLOB ENGINE_SOURCES CONFIGURE_DEPENDS src/engine/*.cpp)
file(GLOB AI_SOURCES CONFIGURE_DEPENDS src/analyzer/*.cpp)
add_library(chess_core STATIC ${ENGINE_SOURCES} ${AI_SOURCES})
target_include_directories(chess_core PUBLIC src)

# ---- console game ----
add_executable(chess src/main.cpp)
target_link_libraries(chess PRIVATE chess_core)

# ---- tests ----
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS tests/test_*.cpp)
add_executable(run_tests tests/run_tests.cpp ${TEST_SOURCES})
target_include_directories(run_tests PRIVATE tests)
target_link_libraries(run_tests PRIVATE chess_core)
enable_testing()
add_test(NAME all_tests COMMAND run_tests)

#pragma once
#include "Types.h"

enum class MoveType { Normal, Castle, EnPassant, Promotion };

struct Move {
    Position from;
    Position to;
    MoveType type = MoveType::Normal;
    PieceType promotion = PieceType::Queen;  // only used for Promotion
};
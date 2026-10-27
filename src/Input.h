#pragma once

#include <raylib.h>

#include "Grid.h"

enum class MouseButtonInput : std::uint8_t
{
    None,
    LeftPressed,
    LeftReleased,
    RightPressed,
};

struct MouseInput
{
    Vector2 pos{};
    MouseButtonInput button = MouseButtonInput::None;
};

struct BoardHit
{
    Cell *cell = nullptr;
    Coords coords{};
};

using MouseTarget = std::variant<std::monostate, BoardHit, Slot *>;

struct Drag
{
    std::optional<Piece> held;
    std::optional<Coords> from;
    Slot *slot;
};
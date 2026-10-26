#pragma once

#include <raylib.h>
#include <raymath.h>

#include <cstdint>
#include <variant>

#include "Color.h"
#include "Def.h"
#include "Light.h"

/**
 *  Overload Pattern
 *  https://www.cppstories.com/2019/02/2lines3featuresoverload.html/
 */
template <class... Fs> struct Overload : Fs...
{
    using Fs::operator()...;
};
template <class... Fs> Overload(Fs...) -> Overload<Fs...>;

enum class MirrorRotation : std::uint8_t
{
    ForwardSlash,
    BackSlash,
};

inline MirrorRotation InvertMirrorDirection(MirrorRotation in)
{
    return in == MirrorRotation::ForwardSlash ? MirrorRotation::BackSlash
                                              : MirrorRotation::ForwardSlash;
}

inline AxisDirection RotateMirrorDirection(AxisDirection in, MirrorRotation rot)
{
    switch (in)
    {
        case AxisDirection::North:
            return rot == MirrorRotation::ForwardSlash ? AxisDirection::West
                                                       : AxisDirection::East;
        case AxisDirection::South:
            return rot == MirrorRotation::ForwardSlash ? AxisDirection::East
                                                       : AxisDirection::West;
        case AxisDirection::East:
            return rot == MirrorRotation::ForwardSlash ? AxisDirection::South
                                                       : AxisDirection::North;
        case AxisDirection::West:
            return rot == MirrorRotation::ForwardSlash ? AxisDirection::North
                                                       : AxisDirection::South;
        default:
            return AxisDirection::None;
    }
}

struct Wall
{
};

struct Source
{
    LightColor color;
    AxisDirection out;
    int strength = 5;
};

struct Target
{
    LightColor color;
};

struct Mirror
{
    MirrorRotation rot;
};

struct Booster
{
    int strength = 0;
};

struct Inverter
{
};

struct Filter
{
    LightColor keep;
};

struct Tint
{
    LightColor add;
};

struct Combiner
{
    AxisDirection out;
    LightColor color; // changes with inputs
};

using Piece = std::variant<std::monostate, Wall, Source, Target, Mirror,
    Booster, Inverter, Filter, Tint, Combiner>;

inline bool IsPieceEmpty(const Piece &p)
{
    return std::holds_alternative<std::monostate>(p);
}

inline void RotatePiece(Piece &p)
{
    std::visit(
        Overload{
            [](Mirror &m) { m.rot = InvertMirrorDirection(m.rot); },
            [](Combiner &c) { c.out = RotateDirection(c.out); },
            [](auto &) {},
        },
        p);
}

inline void DrawPiece(const Piece &piece, Rectangle bounds)
{
    const Rectangle r =
        Inset(bounds, Vector2{ GRID_CELL_SIZE * 0.1F, GRID_CELL_SIZE * 0.1F });
    const Vector2 center{ r.x + (r.width / 2), r.y + (r.height / 2) };
}
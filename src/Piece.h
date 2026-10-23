#pragma once

#include <raymath.h>

#include <cstdint>
#include <variant>

#include "Color.h"

enum class AxisDirection : std::uint8_t
{
    None,
    North,
    South,
    East,
    West,
};

enum class MirrorRotation : std::uint8_t
{
    ForwardSlash,
    BackSlash,
};

struct LightRay
{
    LightColor color = LightColor::None;
    int strength = 0;
    AxisDirection in;
    AxisDirection out;
    bool hasSpread = false;
    [[nodiscard]] bool isLit() const { return color != LightColor::None; }
    friend bool operator==(const LightRay &, const LightRay &) = default;
};

/**
 *  Overload Pattern
 *  https://www.cppstories.com/2019/02/2lines3featuresoverload.html/
 */
template <class... Fs> struct Overload : Fs...
{
    using Fs::operator()...;
};
template <class... Fs> Overload(Fs...) -> Overload<Fs...>;

inline AxisDirection RotateDirection(AxisDirection in, MirrorRotation rot)
{
    switch (in)
    {
        case AxisDirection::North:
            return rot == MirrorRotation::ForwardSlash ? AxisDirection::West : AxisDirection::East;
        case AxisDirection::South:
            return rot == MirrorRotation::ForwardSlash ? AxisDirection::East : AxisDirection::West;
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

// -------
// PIECES
struct Wall
{
};

struct Source
{
    LightColor color;
    AxisDirection out;
    int strength;
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
};

using Piece = std::variant<std::monostate, Wall, Source, Target, Mirror, Booster, Inverter, Filter,
    Tint, Combiner>;

inline bool IsPieceEmpty(const Piece &p)
{
    return std::holds_alternative<std::monostate>(p);
}

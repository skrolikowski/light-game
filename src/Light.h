#pragma once

#include <cstdint>

#include "Color.h"

enum class AxisDirection : std::uint8_t
{
    None,
    North,
    South,
    East,
    West,
};

struct LightRay
{
    LightColor color = LightColor::None;
    int strength = 0;
    int strengthMax = 0;
    AxisDirection in = AxisDirection::None;
    AxisDirection out = AxisDirection::None;
    bool hasSpread = false;
    friend bool operator==(const LightRay &, const LightRay &) = default;
};

inline AxisDirection InvertDirection(AxisDirection in)
{
    switch (in)
    {
        case AxisDirection::North:
            return AxisDirection::South;
        case AxisDirection::South:
            return AxisDirection::North;
        case AxisDirection::East:
            return AxisDirection::West;
        case AxisDirection::West:
            return AxisDirection::East;
        default:
            return AxisDirection::None;
    }
}

inline AxisDirection RotateDirection(AxisDirection in)
{
    switch (in)
    {
        case AxisDirection::North:
            return AxisDirection::West;
        case AxisDirection::South:
            return AxisDirection::East;
        case AxisDirection::East:
            return AxisDirection::North;
        case AxisDirection::West:
            return AxisDirection::South;
        default:
            return AxisDirection::None;
    }
}

inline float DirectionToAngle(AxisDirection dir)
{
    switch (dir)
    {
        case AxisDirection::South:
            return 90.F;
        case AxisDirection::West:
            return 180.F;
        case AxisDirection::North:
            return 270.F;
        default:
            return 0.F;
    }
}
#pragma once

#include <algorithm>
#include <cstdint>
#include <raylib.h>

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

inline Color ToRaylibColor(LightColor color)
{
    switch (color)
    {
        case LightColor::Red:
            return RED;
        case LightColor::Green:
            return GREEN;
        case LightColor::Blue:
            return BLUE;
        case LightColor::Yellow:
            return YELLOW;
        case LightColor::Magenta:
            return MAGENTA;
        case LightColor::Cyan:
            return SKYBLUE;
        case LightColor::White:
            return WHITE;
        default:
            return BLACK;
    }
}

inline void DrawLightRay(const LightRay &ray, Rectangle bounds)
{
    const Vector2 center{ bounds.x + (bounds.width / 2),
        bounds.y + (bounds.height / 2) };

    // Edge midpoint for a direction (center if None)..
    auto edge = [&](AxisDirection dir) -> Vector2
    {
        switch (dir)
        {
            case AxisDirection::North:
                return { center.x, bounds.y };
            case AxisDirection::South:
                return { center.x, bounds.y + bounds.height };
            case AxisDirection::East:
                return { bounds.x + bounds.width, center.y };
            case AxisDirection::West:
                return { bounds.x, center.y };
            default:
                return center;
        }
    };
    const Vector2 in = edge(ray.in);
    const Vector2 out = edge(ray.out);

    // Weak rays fade, but never vanish entirely..
    float strength = std::clamp(
        static_cast<float>(ray.strength / ray.strengthMax), 0.25F, 1.F);
    float pulseBase = 0.9F;
    float pulseAmp = 0.1F;
    float pulseFreq = 4.F;
    strength *= pulseBase + (pulseAmp * sinf(GetTime() * pulseFreq));
    const Color baseColor = ToRaylibColor(ray.color);

    // Rounded capsule: in => center => out
    auto stroke = [&](float thick, Color c)
    {
        DrawLineEx(in, center, thick, c);
        DrawLineEx(center, out, thick, c);
        DrawCircleV(center, thick / 2, c);
    };

    // Soft glow (wide + faint => narrow + bright), then a hot core..
    stroke(32.F, Fade(baseColor, 0.10F * strength));
    stroke(24.F, Fade(baseColor, 0.25F * strength));
    stroke(18.F, Fade(baseColor, 0.40F * strength));
    stroke(12.F, Fade(baseColor, 0.60F * strength));
    stroke(8.F, Fade(ColorBrightness(baseColor, 0.5F), 0.75F * strength));
}
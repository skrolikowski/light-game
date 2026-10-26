#pragma once

#include <cstdint>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include <variant>
#include <vector>

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

inline void TransformPiece(const Piece &piece, std::vector<LightRay> &rays)
{
    auto each = [&](auto fn)
    {
        for (LightRay &r : rays)
        {
            if (r.hasSpread || r.strength == 0)
                continue;
            fn(r);
        }
    };

    std::visit(
        Overload{
            [](const Wall &) { /* absorbs light */ },
            [&](const Source &o)
            {
                LightRay source = {
                    .color = o.color,
                    .strength = o.strength,
                    .strengthMax = o.strength,
                    .out = o.out,
                };
                rays.push_back(source);
            },
            [](const Target &) { /* absorbs light */ },
            [&](const Mirror &o)
            {
                each([&](LightRay &r)
                    { r.out = RotateMirrorDirection(r.in, o.rot); });
            },
            [&](const Booster &o)
            {
                each(
                    [&](LightRay &r)
                    {
                        r.strength =
                            std::min(r.strength + o.strength, RAY_STRENGTH_MAX);
                        r.out = InvertDirection(r.in);
                    });
            },
            [&](const Inverter &)
            {
                each(
                    [](LightRay &r)
                    {
                        r.color = invert(r.color);
                        r.out = InvertDirection(r.in);
                    });
            },
            [&](const Filter &o)
            {
                each(
                    [&](LightRay &r)
                    {
                        r.color = r.color & o.keep;
                        r.out = InvertDirection(r.in);
                    });
            },
            [&](const Tint &o)
            {
                each(
                    [&](LightRay &r)
                    {
                        r.color = r.color | o.add;
                        r.out = InvertDirection(r.in);
                    });
            },
            [&](auto &)
            {
                // Pass-thru..
                each([](LightRay &r) { r.out = InvertDirection(r.in); });
            },
        },
        piece);
}

inline void DrawPiece(const Piece &piece, Rectangle bounds)
{
    const Vector2 pad{ GRID_CELL_SIZE * 0.25F, GRID_CELL_SIZE * 0.25F };
    const Rectangle r = Inset(bounds, pad);
    const Vector2 center{ r.x + (r.width / 2), r.y + (r.height / 2) };

    std::visit(
        Overload{
            [](std::monostate) { /*Draw nothing*/ },
            [&](const Wall &)
            {
                DrawRectangleRoundedLinesEx(r,0.2F,20,2.F,LIGHTGRAY);
                DrawRectangleRounded(r, 0.2F, 20, DARKGRAY);
            },
            [&](const Source &o) 
            {
                float wedge = 45.F;
                const float facing = DirectionToAngle(o.out);
                const float startAngle = facing + wedge;
                const float endAngle = facing + 270 + wedge;
                float radMin = r.width * 0.5F;
                float radMax = r.width * 0.8F;
                Color baseColor = ToRaylibColor(o.color);
                auto stroke = [&](float innerRadius, float outerRadius, Color c)
                {
                    DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, 0, c);
                };
                // float rad =
                //     radMin + ((radMax - radMin) * static_cast<float>(o.strength) /
                //                 RAY_STRENGTH_MAX);

                // DrawCircleSector(center, rad, facing - wedge, facing + wedge, 0, ToRaylibColor(o.color));
            
                stroke(r.width * 0.6F, r.width * 0.8F, Fade(baseColor, 0.10F));
                stroke(r.width * 0.5F, r.width * 0.6F, Fade(baseColor, 0.25F));
                stroke(r.width * 0.4F, r.width * 0.5F, Fade(baseColor, 0.5F));
            },
            [&](const Target &o)
            {
        float radMin = r.width / 4.F;
        float radMax = r.width / 3.F;
        DrawRing(center, radMin, radMax, 0, 360, 32, ToRaylibColor(o.color));
            },
            [&](const Mirror &o)
            {
        DrawRectangleRoundedLinesEx(r, 0.2F, 20, 4.F, BLACK);

        const bool fwd = o.rot == MirrorRotation::ForwardSlash;
        Vector2 a{ r.x, fwd ? r.y + r.height : r.y };
        Vector2 b{ r.x + r.width, fwd ? r.y : r.y + r.height };
        DrawLineEx(a, b, 4.F, BLACK);
            },
            [&](const Booster &o)
            {
        // if (o.out == AxisDirection::None)
        // {
        //     DrawCircleV(center, rad, ToRaylibColor(o.color));
        //     return;
        // }
        DrawRectangleRoundedLinesEx(r, 0.2F, 20, 4.F, BLACK);
        std::string s = "+" + std::to_string(o.strength);
        DrawText(s.c_str(), center.x, center.y, 10, WHITE);
            },
            [&](const Inverter &)
            {
                
            },
            [&](const Filter &o)
            {
        DrawRing(center, r.width / 4, r.width / 3, 0, 360, 32,
            ToRaylibColor(o.keep));
            },
            [&](const Tint &o)
            {
        DrawCircleV(center, r.width / 3, ToRaylibColor(o.add));
            },
            [&](const Combiner &o)
            {
        // TODO: Rounded Rectangle Border
        // TODO: Tiny o.color Triangle pointing in out direction
        //  Triangle pointing East, relative to center; rotated to face o.out
        const float h = r.width / 2.F;
        const float angle = DirectionToAngle(o.out) * DEG2RAD;
        const Vector2 tip =
            Vector2Add(center, Vector2Rotate({ h, 0.F }, angle));
        const Vector2 baseA =
            Vector2Add(center, Vector2Rotate({ -h, -h }, angle));
        const Vector2 baseB =
            Vector2Add(center, Vector2Rotate({ -h, h }, angle));
        DrawTriangle(
            tip, baseA, baseB, BLACK); // raylib needs counter-clockwise order
            },
            // [](auto&) {},
        },
        piece);
}
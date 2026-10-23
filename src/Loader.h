#pragma once

#include <fstream>
#include <raylib.h>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Color.h"
#include "Piece.h"

inline void LoadLevel(const std::string &path, json &data)
{
    std::ifstream file(path);
    if (!file)
    {
        std::fprintf(stderr, "Unable to open %s for reading.", path.c_str());
    }

    try
    {
        data = nlohmann::json::parse(file);
    }
    catch (nlohmann::json::parse_error &exception)
    {
        std::fprintf(stderr, "Error parsing JSON from `%s`: %s.", path.c_str(), exception.what());
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

inline LightColor ReadColor(const std::string &name)
{
    if (name == "red")
        return LightColor::Red;
    if (name == "green")
        return LightColor::Green;
    if (name == "blue")
        return LightColor::Blue;
    if (name == "yellow")
        return LightColor::Yellow;
    if (name == "magenta")
        return LightColor::Magenta;
    if (name == "cyan")
        return LightColor::Cyan;
    if (name == "white")
        return LightColor::White;

    return LightColor::None;
}

inline AxisDirection ReadAxisDirection(const std::string &d)
{
    if (d == "north")
        return AxisDirection::North;
    if (d == "south")
        return AxisDirection::South;
    if (d == "east")
        return AxisDirection::East;
    if (d == "west")
        return AxisDirection::West;

    return AxisDirection::None;
}

inline Piece ReadPiece(const json &data)
{
    std::string type = data.value("type", "none");

    if (type == "wall")
        return Wall{};

    if (type == "source")
    {
        const LightColor color = ReadColor(data.value("color", "white"));
        const AxisDirection dir = ReadAxisDirection(data.value("direction", "none"));
        int strength = data.value("strength", 5);
        return Source{ .color = color, .out = dir, .strength = strength };
    }

    if (type == "target")
    {
        const LightColor color = ReadColor(data.value("color", "white"));
        return Target{ .color = color };
    }

    if (type == "mirror")
    {
        std::string rot = data.value("rot", "/");
        if (rot == "/")
            return Mirror{ .rot = MirrorRotation::ForwardSlash };
        return Mirror{ .rot = MirrorRotation::BackSlash };
    }

    if (type == "booster")
    {
        int strength = data.value("strength", 5);
        return Booster{ .strength = strength };
    }

    if (type == "inverter")
        return Inverter{};

    if (type == "filter")
    {
        const LightColor color = ReadColor(data.value("color", "white"));
        return Filter{ .keep = color };
    }

    if (type == "tint")
    {
        const LightColor color = ReadColor(data.value("color", "white"));
        return Tint{ .add = color };
    }

    if (type == "combiner")
    {
        const AxisDirection dir = ReadAxisDirection(data.value("direction", "none"));
        return Combiner{ .out = dir };
    }

    throw std::runtime_error("unknown color");
}

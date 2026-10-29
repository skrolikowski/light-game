#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

#include "Color.h"
#include "Piece.h"

namespace Loader
{
inline void LoadFileData(const std::string &path, json &data)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + path);

    try
    {
        data = nlohmann::json::parse(file);
    }
    catch (nlohmann::json::parse_error &exception)
    {
        throw std::runtime_error("Error parsing JSON from file: " + path);
    }
}

inline std::size_t GetPieceCount(json &data)
{
    return data.at("pieces").size();
}

inline std::size_t GetInventoryPieceCount(json &data, const std::string &name)
{
    std::size_t count = 0;

    const json &invData = data.at("inventory");
    const int numPieces = static_cast<int>(invData.size());
    for (int i = 0; i < numPieces; ++i)
    {
        const json &pieceData = invData.at(static_cast<std::size_t>(i));
        std::string pieceType = pieceData.at("type");
        std::size_t pieceCount = pieceData.at("count");
        if (pieceType == name)
        {
            count += pieceCount;
        }
    }

    return count;
}

inline std::string PieceToString(const Piece &p)
{
    if (std::holds_alternative<Wall>(p))
        return "wall";
    if (std::holds_alternative<Source>(p))
        return "source";
    if (std::holds_alternative<Target>(p))
        return "target";
    if (std::holds_alternative<Mirror>(p))
        return "mirror";
    if (std::holds_alternative<Booster>(p))
        return "booster";
    if (std::holds_alternative<Inverter>(p))
        return "inverter";
    if (std::holds_alternative<Filter>(p))
        return "filter";
    if (std::holds_alternative<Tint>(p))
        return "tint";
    if (std::holds_alternative<Combiner>(p))
        return "combiner";
    return "none";
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
    std::string type = data.at("type");

    if (type == "wall")
        return Wall{};

    if (type == "source")
        return Source{
            .color = ReadColor(data.at("color")),
            .out = ReadAxisDirection(data.at("direction")),
            .strength = data.at("strength"),
        };

    if (type == "target")
        return Target{ .color = ReadColor(data.at("color")) };

    if (type == "mirror")
    {
        std::string rot = data.at("rot");
        if (rot == "/")
            return Mirror{ .rot = MirrorRotation::ForwardSlash };
        return Mirror{ .rot = MirrorRotation::BackSlash };
    }

    if (type == "booster")
        return Booster{ .strength = data.at("strength") };

    if (type == "inverter")
        return Inverter{};

    if (type == "filter")
        return Filter{ .keep = ReadColor(data.at("color")) };

    if (type == "tint")
        return Tint{ .add = ReadColor(data.at("color")) };

    if (type == "combiner")
        return Combiner{ .out = ReadAxisDirection(data.at("out")) };

    return std::monostate{};
}
}; // namespace Loader

#include <algorithm>
#include <nlohmann/json-schema.hpp>

using nlohmann::json_schema::json_validator;

#include "Level.h"

Level::Level(const json &data)
{
    // validate against schema
    json schema;
    LoadFileData("res/levels/level.schema.json", schema);

    json_validator validator;
    try
    {
        validator.set_root_schema(schema);
        validator.validate(data);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(e.what());
    }

    levelData_ = data;

    title_ = levelData_.value("title", "Default Title");
    rows_ = levelData_.at("grid").value("rows", 0);
    cols_ = levelData_.at("grid").value("cols", 0);

    parsePieces();
    parseSlots();
}

void Level::parsePieces()
{
    const json &pieces = levelData_.at("pieces");
    const int numPieces = static_cast<int>(pieces.size());
    for (int i = 0; i < numPieces; ++i)
    {
        const json &pieceData = pieces.at(static_cast<unsigned long>(i));
        Piece piece = ReadPiece(pieceData);

        if (pieceData.contains("coords"))
        {
            int row = pieceData.at("coords").value("row", 0);
            int col = pieceData.at("coords").value("col", 0);

            if (row >= 0 and row < rows_ and col >= 0 and col < cols_)
            {
                Coords coords = { .row = row, .col = col };

                // Check if piece is at coords already..
                if (!pieces_.contains(coords))
                {
                    pieces_[coords] = piece;
                }
            }
        }
    }
}

void Level::parseSlots()
{
    const json &inventory = levelData_.at("inventory");
    const int numSlots = static_cast<int>(inventory.size());

    for (int i = 0; i < numSlots; ++i)
    {
        const json &invData = inventory.at(static_cast<std::size_t>(i));
        Piece piece = ReadPiece(invData);
        int pieceCount = invData.at("count");

        auto it = std::ranges::find_if(slots_,
            [&](const auto &pair) { return piece == pair.second.piece; });
        if (it != slots_.end())
        {
            it->second.count += pieceCount;
        }
        else
        {
            Coords coords{ .row = static_cast<int>(slots_.size()), .col = 0 };
            slots_[coords] = { .piece = piece, .count = pieceCount };
        }
    }
}
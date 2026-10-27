#include "Level.h"

#include <algorithm>

Level::Level(std::string &path) : path_(path)
{
    LoadLevelData(path, levelData_);

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
            Coords coords = { .row = row, .col = col };
            peices_[coords] = piece;
        }
    }
}

void Level::parseSlots()
{
    const json &inventory = levelData_.at("inventory");
    const int numSlots = static_cast<int>(inventory.size());

    for (int i = 0; i < numSlots; ++i)
    {
        const json &invData = inventory.at(i);
        Piece piece = ReadPiece(invData);
        int pieceCount = invData.value("count", 1);

        auto it = std::ranges::find_if(slots_,
            [&](const auto &pair) { return piece == pair.second.piece; });
        if (it != slots_.end())
        {
            it->second.count += pieceCount;
        }
        else
        {
            Coords coords{ .row = i, .col = 0 };
            slots_[coords] = { .piece = piece, .count = pieceCount };
        }
    }
}
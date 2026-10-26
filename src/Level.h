#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Grid.h"
#include "Loader.h"

class Level
{
  public:
    Level() = default;
    Level(std::string &path) : path_(path)
    {
        LoadLevelData(path, levelData_);

        title_ = levelData_.value("title", "Default Title");
        rows_ = levelData_.at("grid").value("rows", 0);
        cols_ = levelData_.at("grid").value("cols", 0);

        parsePieces();
        parseSlots();
    };

    std::map<Coords, Piece> GetBoardPieces() { return peices_; }
    std::map<Coords, Slot> GetSlotPieces() { return slots_; }

    [[nodiscard]] std::string GetTitle() const noexcept { return title_; }
    [[nodiscard]] int GetRows() const noexcept { return rows_; }
    [[nodiscard]] int GetCols() const noexcept { return cols_; }

  private:
    json levelData_ = json::object();

    std::string title_;
    std::string path_;
    int rows_;
    int cols_;
    std::map<Coords, Piece> peices_;
    std::map<Coords, Slot> slots_;

    void parsePieces()
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

    void parseSlots()
    {
        const json &inventory = levelData_.at("inventory");
        const int numSlots = static_cast<int>(inventory.size());

        for (int i = 0; i < numSlots; ++i)
        {
            const json &invData = inventory.at(i);
            Piece piece = ReadPiece(invData);
            int pieceCount = invData.value("count", 1);
            Coords coords{ .row = i, .col = 0 };
            slots_[coords] = { .piece = piece, .count = pieceCount };
        }
    }
};
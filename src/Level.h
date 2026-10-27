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
    Level(std::string path);

    std::map<Coords, Piece> GetBoardPieces() { return peices_; }
    std::map<Coords, Slot> GetSlotPieces() { return slots_; }

    [[nodiscard]] std::string GetTitle() const noexcept { return title_; }
    [[nodiscard]] int GetRows() const noexcept { return rows_; }
    [[nodiscard]] int GetCols() const noexcept { return cols_; }

  private:
    json levelData_ = json::object();

    std::string title_;
    std::string path_;
    int rows_ = 0;
    int cols_ = 0;
    std::map<Coords, Piece> peices_;
    std::map<Coords, Slot> slots_;

    void parsePieces();
    void parseSlots();
};
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
    Level(const json &data);
    Level(const std::string &path) : Level(ReadFile(path)) {}

    [[nodiscard]] const std::map<Coords, Piece> &GetBoardPieces() const noexcept
    {
        return pieces_;
    }
    [[nodiscard]] const std::map<Coords, Slot> &GetSlotPieces() const noexcept
    {
        return slots_;
    }

    [[nodiscard]] std::string GetTitle() const noexcept { return title_; }
    [[nodiscard]] int GetRows() const noexcept { return rows_; }
    [[nodiscard]] int GetCols() const noexcept { return cols_; }

    static json ReadFile(const std::string &path)
    {
        json d;
        LoadFileData(path, d);
        return d;
    }

  private:
    std::string title_;
    int rows_ = 0;
    int cols_ = 0;
    std::map<Coords, Piece> pieces_;
    std::map<Coords, Slot> slots_;

    void parsePieces(const json &data);
    void parseSlots(const json &data);
};
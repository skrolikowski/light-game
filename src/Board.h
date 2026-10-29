#pragma once

#include <map>
#include <raylib.h>

#include "Grid.h"

// Forward declarations..
struct LightField;
struct LightFieldResult;

using CellGrid = Grid<Cell>;

class Board
{
  public:
    Board() = default;
    Board(int rows, int cols, const Vector2 &size);
    Board(const Vector2 &origin, int rows, int cols, const Vector2 &size);

    [[nodiscard]] int GetNumPieces() const noexcept;
    [[nodiscard]] int GetNumCols() const noexcept;
    [[nodiscard]] int GetNumRows() const noexcept;
    [[nodiscard]] const Vector2 &GetSize() const noexcept;
    [[nodiscard]] Piece &GetPieceAt(Coords coords) const;
    [[nodiscard]] Piece &GetPieceAt(Vector2 v) const;
    [[nodiscard]] Cell *GetCellAt(Coords coords) const;
    [[nodiscard]] Cell *GetCellAt(Vector2 v) const;
    [[nodiscard]] Coords GetCoordsAt(Vector2 v) const;
    [[nodiscard]] Rectangle GetRectAt(Coords coords) const;

    void AddPieces(const std::map<Coords, Piece> &pieces) const;
    void CheckStability(const LightField &field, LightFieldResult &out);
    void Draw() const { grid_->Draw(); }

    [[nodiscard]] bool InBounds(Coords coords) const noexcept
    {
        return coords.row >= 0 && coords.row < GetNumRows() &&
               coords.col >= 0 && coords.col < GetNumCols();
    }

    void DrawPieces() const
    {
        grid_->ForEach([&](const Cell &cell, Coords coords)
            { DrawPiece(cell.piece, GetRectAt(coords)); });
    }

  private:
    std::unique_ptr<CellGrid> grid_;
};
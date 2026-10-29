#include "Board.h"
#include "Game.h"

Board::Board(int rows, int cols, const Vector2 &size)
{
    grid_ = std::make_unique<CellGrid>(CellGrid{ rows, cols, size });
}

Board::Board(const Vector2 &origin, int rows, int cols, const Vector2 &size)
{
    grid_ = std::make_unique<CellGrid>(CellGrid{ origin, rows, cols, size });
}

void Board::AddPieces(const std::map<Coords, Piece> &pieces) const
{
    for (auto [coords, piece] : pieces)
        GetPieceAt(coords) = piece;
}

int Board::GetNumPieces() const noexcept
{
    int count = 0;
    grid_->ForEach([&](const Cell &cell, Coords)
        { count += IsPieceEmpty(cell.piece) ? 0 : 1; });
    return count;
}

int Board::GetNumCols() const noexcept
{
    return grid_->GetNumCols();
}
int Board::GetNumRows() const noexcept
{
    return grid_->GetNumRows();
}
const Vector2 &Board::GetSize() const noexcept
{
    return grid_->GetSize();
}

Piece &Board::GetPieceAt(Coords coords) const
{
    return grid_->GetItemAt(coords)->piece;
}

Piece &Board::GetPieceAt(Vector2 v) const
{
    return grid_->GetItemAt(v)->piece;
}

Cell *Board::GetCellAt(Coords coords) const
{
    return grid_->GetItemAt(coords);
}

Cell *Board::GetCellAt(Vector2 v) const
{
    return grid_->GetItemAt(v);
}

Coords Board::GetCoordsAt(Vector2 v) const
{
    return grid_->GetCoordsAt(v);
}

Rectangle Board::GetRectAt(Coords coords) const
{
    return grid_->GetRectAt(coords);
}

void Board::CheckStability(const LightField &field, LightFieldResult &out)
{
    grid_->ForEach(
        [&](const Cell &cell, Coords coords)
        {
            const auto *targetPiece = std::get_if<Target>(&cell.piece);
            if (targetPiece != nullptr)
            {
                out.numTargets++;
                std::vector<LightRay> rays =
                    field.grid->GetItemAt(coords)->rays;

                for (const auto &ray : rays)
                    if (ray.strength > 0 && ray.color == targetPiece->color)
                        out.numTargetsLit++;
            }
        });
}
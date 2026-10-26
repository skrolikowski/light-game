#pragma once

#include <map>
#include <optional>
#include <raylib.h>
#include <vector>

#include "Piece.h"

struct Coords
{
    int row = 0;
    int col = 0;

    Coords operator+(Coords o) const
    {
        return { .row = row + o.row, .col = col + o.col };
    }
    Coords operator+=(Coords o)
    {
        row += o.row;
        col += o.col;
        return *this;
    }
    auto operator<=>(const Coords &) const = default;
};

struct Slot
{
    Piece piece;
    int count = 0;
};

struct Cell
{
    Piece piece;
    Slot *slot = nullptr;
    [[nodiscard]] bool isMovable() const { return slot != nullptr; }

    friend bool operator==(const Cell &, const Cell &) = default;
};

inline Coords Translate(AxisDirection dir, Coords coords)
{
    switch (dir)
    {
        case AxisDirection::North:
            coords += Coords{ .row = -1, .col = 0 };
            break;
        case AxisDirection::South:
            coords += Coords{ .row = 1, .col = 0 };
            break;
        case AxisDirection::East:
            coords += Coords{ .row = 0, .col = 1 };
            break;
        case AxisDirection::West:
            coords += Coords{ .row = 0, .col = -1 };
            break;
        default:
            break;
    }
    return coords;
}

inline Rectangle Inset(Rectangle r, Vector2 pad)
{
    return { r.x + pad.x, r.y + pad.y, r.width - (2 * pad.x),
        r.height - (2 * pad.y) };
}

template <typename T> class Grid
{
  public:
    Grid() = default;
    Grid(int rows, int cols, Vector2 size)
        : rows_(rows), cols_(cols), size_(size)
    {
        createGrid();
    }
    Grid(Vector2 origin, int rows, int cols, Vector2 size)
        : origin_(origin), rows_(rows), cols_(cols), size_(size)
    {
        createGrid();
    }

    [[nodiscard]] const T *GetItem(Coords coords) const noexcept;
    [[nodiscard]] T *GetItem(Coords coords) noexcept;
    [[nodiscard]] const T *GetItemAt(Vector2 v) const noexcept;
    [[nodiscard]] T *GetItemAt(Vector2 v) noexcept;
    [[nodiscard]] Vector2 GetSize() const noexcept { return size_; }
    [[nodiscard]] Coords GetCoordsAt(Vector2) const noexcept;
    [[nodiscard]] Vector2 GetTopLeft(Coords coords) const noexcept;
    [[nodiscard]] Rectangle GetRect(Coords coords) const noexcept;

    [[nodiscard]] int NumCols() const noexcept { return cols_; }
    [[nodiscard]] int NumRows() const noexcept { return rows_; }

    static bool InBounds(const Grid *grid, Coords coords)
    {
        return coords.row >= 0 && coords.row < grid->rows_ && coords.col >= 0 &&
               coords.col < grid->cols_;
    }

    static bool InGrid(const Grid *grid, Vector2 v)
    {
        return v.x >= grid->origin_.x && v.y >= grid->origin_.y &&
               v.x < grid->origin_.x + grid->size_.x * grid->cols_ &&
               v.y < grid->origin_.y + grid->size_.y * grid->rows_;
    }

    template <typename F> void ForEach(F &&f)
    {
        for (std::size_t i = 0; i < items_.size(); ++i)
            f(items_[i], convertIdexToCoords(i));
    }

    template <typename F> void ForEach(F &&f) const
    {
        for (std::size_t i = 0; i < items_.size(); ++i)
            f(items_[i], convertIdexToCoords(i));
    }

    void Draw() const;

    friend bool operator==(const Grid &, const Grid &) = default;

  private:
    std::vector<T> items_{};
    Vector2 origin_{};
    int rows_ = 0;
    int cols_ = 0;
    Vector2 size_{};

    void createGrid();

    [[nodiscard]] std::size_t convertCoordsToIndex(Coords coords) const
    {
        return static_cast<std::size_t>(coords.col + (coords.row * cols_));
    }

    [[nodiscard]] Coords convertIdexToCoords(std::size_t index) const noexcept
    {
        return {
            .row = static_cast<int>(index / static_cast<std::size_t>(cols_)),
            .col = static_cast<int>(index % static_cast<std::size_t>(cols_)),
        };
    }
};

using LightRays = std::vector<LightRay>;
using CombinerOutputs = std::map<Coords, LightRay>;

struct LightCell
{
    std::vector<LightRay> rays;
};

struct LightField
{
    std::unique_ptr<Grid<LightCell>> grid;
    bool isStable = true;
};

#include "Grid.tpp"
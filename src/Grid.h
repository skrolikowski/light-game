#pragma once

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

struct LightCell
{
    std::vector<LightRay> rays;
};

template <typename T> class Grid
{
  public:
    Grid() = default;
    Grid(int rows, int cols, Vector2 size);
    Grid(Vector2 origin, int rows, int cols, Vector2 size);

    [[nodiscard]] const T *GetItemAt(Coords coords) const noexcept;
    [[nodiscard]] T *GetItemAt(Coords coords) noexcept;
    [[nodiscard]] const T *GetItemAt(Vector2 v) const noexcept;
    [[nodiscard]] T *GetItemAt(Vector2 v) noexcept;
    [[nodiscard]] const Vector2 &GetSize() const noexcept { return size_; }
    [[nodiscard]] Coords GetCoordsAt(Vector2) const noexcept;
    [[nodiscard]] Vector2 GetTopLeft(Coords coords) const noexcept;
    [[nodiscard]] Rectangle GetRectAt(Coords coords) const noexcept;

    [[nodiscard]] int GetNumCols() const noexcept { return cols_; }
    [[nodiscard]] int GetNumRows() const noexcept { return rows_; }

    static bool InBounds(const Grid *grid, Coords coords);
    static bool InGrid(const Grid *grid, Vector2 v);
    static Coords Translate(AxisDirection dir, Coords coords);

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

#include "Grid.tpp"
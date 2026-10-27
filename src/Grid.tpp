#pragma once

#include <cassert>
#include <cmath>

#include "Grid.h"

template <typename T>
Grid<T>::Grid(int rows, int cols, Vector2 size)
    : rows_(rows), cols_(cols), size_(size)
{
    createGrid();
}
template <typename T>
Grid<T>::Grid(Vector2 origin, int rows, int cols, Vector2 size)
    : origin_(origin), rows_(rows), cols_(cols), size_(size)
{
    createGrid();
}

template <typename T> void Grid<T>::createGrid()
{
    items_.reserve(static_cast<std::size_t>(cols_ * rows_));

    for (int y = 0; y < rows_; ++y)
    {
        for (int x = 0; x < cols_; ++x)
        {
            T item{};
            items_.push_back(item);
        }
    }
}

template <typename T> Coords Grid<T>::GetCoordsAt(Vector2 v) const noexcept
{
    return {
        .row = static_cast<int>(std::floor((v.y - origin_.y) / size_.y)),
        .col = static_cast<int>(std::floor((v.x - origin_.x) / size_.x)),
    };
}

template <typename T> const T *Grid<T>::GetItemAt(Vector2 v) const noexcept
{
    return GetItem(GetCoordsAt(v));
}

template <typename T> T *Grid<T>::GetItemAt(Vector2 v) noexcept
{
    return GetItem(GetCoordsAt(v));
}

template <typename T> Vector2 Grid<T>::GetTopLeft(Coords coords) const noexcept
{
    return {
        .x = origin_.x + (static_cast<float>(coords.col) * size_.x),
        .y = origin_.y + (static_cast<float>(coords.row) * size_.y),
    };
}

template <typename T> Rectangle Grid<T>::GetRect(Coords coords) const noexcept
{
    Vector2 topLeft = GetTopLeft(coords);
    return {
        .x = topLeft.x, .y = topLeft.y, .width = size_.x, .height = size_.y
    };
}

template <typename T> const T *Grid<T>::GetItem(Coords coords) const noexcept
{
    if (!InBounds(this, coords))
        return nullptr;
    return &items_[convertCoordsToIndex(coords)];
}

template <typename T> T *Grid<T>::GetItem(Coords coords) noexcept
{
    if (!InBounds(this, coords))
        return nullptr;
    return &items_[convertCoordsToIndex(coords)];
}

template <typename T> void Grid<T>::Draw() const
{
    ForEach([&](const T &, Coords coords)
        { DrawRectangleLinesEx(GetRect(coords), 1.F, LIGHTGRAY); });
}

template <typename T> bool Grid<T>::InBounds(const Grid *grid, Coords coords)
{
    return coords.row >= 0 && coords.row < grid->rows_ && coords.col >= 0 &&
           coords.col < grid->cols_;
}

template <typename T> bool Grid<T>::InGrid(const Grid *grid, Vector2 v)
{
    return v.x >= grid->origin_.x && v.y >= grid->origin_.y &&
           v.x < grid->origin_.x + grid->size_.x * grid->cols_ &&
           v.y < grid->origin_.y + grid->size_.y * grid->rows_;
}

template <typename T>
Coords Grid<T>::Translate(AxisDirection dir, Coords coords)
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
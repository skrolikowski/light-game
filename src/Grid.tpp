#pragma once

#include <cassert>

#include "Grid.h"

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
        .row = static_cast<int>((v.y - origin_.y) / size_.y),
        .col = static_cast<int>((v.x - origin_.x) / size_.x),
    };
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
    return { .x = topLeft.x, .y = topLeft.y, .width = size_.x, .height = size_.y };
}

template <typename T> const T &Grid<T>::GetItem(Coords coords) const
{
    if (!InBounds(this, coords))
        throw std::out_of_range("Grid coordinate out of range");
    return items_.at(convertCoordsToIndex(coords));
}

template <typename T> T &Grid<T>::GetItem(Coords coords)
{
    if (!InBounds(this, coords))
        throw std::out_of_range("Grid coordinate out of range");
    return items_.at(convertCoordsToIndex(coords));
}

template <typename T> void Grid<T>::Draw() const
{
    ForEach([&](const Cell &, Coords coords)
        { DrawRectangleLinesEx(GetRect(coords), 1.F, LIGHTGRAY); });
}
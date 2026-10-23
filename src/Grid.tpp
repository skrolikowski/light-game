#pragma once

#include <cassert>

#include "Def.h"
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

template <typename T> const T &Grid<T>::GetItem(Coords coords) const
{
    if (!isInBounds(coords))
        throw std::out_of_range("Grid coordinate out of range");
    return items_.at(convertCoordsToIndex(coords));
}

template <typename T> T &Grid<T>::GetItem(Coords coords)
{
    if (!isInBounds(coords))
        throw std::out_of_range("Grid coordinate out of range");
    return items_.at(convertCoordsToIndex(coords));
}

template <typename T> void Grid<T>::Draw() const
{
    float lineThickness = 1.F;
    auto gridColor = LIGHTGRAY;

    for (int r = 0; r < rows_; ++r)
    {
        for (int c = 0; c < cols_; ++c)
        {
            Rectangle rect{
                .x = static_cast<float>(c) * GRID_CELL_SIZE + origin_.x,
                .y = static_cast<float>(r) * GRID_CELL_SIZE + origin_.y,
                .width = GRID_CELL_SIZE,
                .height = GRID_CELL_SIZE,
            };
            DrawRectangleLinesEx(rect, lineThickness, gridColor);
        }
    }
}
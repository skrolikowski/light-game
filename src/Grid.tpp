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
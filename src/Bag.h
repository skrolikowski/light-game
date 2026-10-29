#pragma once

#include <map>
#include <raylib.h>

#include "Grid.h"

using SlotGrid = Grid<Slot>;

class Bag
{
  public:
    Bag() = default;
    Bag(int rows, int cols, const Vector2 &size);
    Bag(const Vector2 &origin, int rows, int cols, const Vector2 &size);

    [[nodiscard]] Slot *GetSlotAt(Coords coords) const;
    [[nodiscard]] Slot *GetSlotAt(Vector2 v) const;

    [[nodiscard]] std::map<Piece *, int> GetPieces() const;
    void AddPieces(std::map<Coords, Slot> &slots);
    void Draw() const;

  private:
    static void drawInventorySlot(const Slot &slot, Rectangle bounds);

    std::unique_ptr<SlotGrid> grid_;
};
#include "Bag.h"

Bag::Bag(int rows, int cols, const Vector2 &size)
{
    grid_ = std::make_unique<SlotGrid>(SlotGrid{ rows, cols, size });
}
Bag::Bag(const Vector2 &origin, int rows, int cols, const Vector2 &size)
{
    grid_ = std::make_unique<SlotGrid>(SlotGrid{ origin, rows, cols, size });
}

Slot *Bag::GetSlotAt(Coords coords) const
{
    return grid_->GetItemAt(coords);
}

Slot *Bag::GetSlotAt(Vector2 v) const
{
    return grid_->GetItemAt(v);
}

std::map<Piece *, int> Bag::GetPieces() const
{
    std::map<Piece *, int> out;
    grid_->ForEach(
        [&](Slot &slot, Coords coords) { out[&slot.piece] = slot.count; });
    return out;
}

void Bag::AddPieces(std::map<Coords, Slot> &slots)
{
    grid_->ForEach(
        [&](Slot &slot, Coords coords)
        {
            if (slots.contains(coords))
                slot = slots.at(coords);
        });
}

void Bag::Draw() const
{
    grid_->ForEach([&](const Slot &slot, Coords coords)
        { drawInventorySlot(slot, grid_->GetRectAt(coords)); });
}

void Bag::drawInventorySlot(const Slot &slot, Rectangle bounds)
{
    DrawRectangleRoundedLinesEx(bounds, 0.2F, 20, 1.F, BLACK);
    DrawRectangleRounded(bounds, 0.2F, 20, (slot.count > 0 ? WHITE : DARKGRAY));

    // Draw Slot Piece
    Rectangle icon = Inset(
        Rectangle{
            .x = bounds.x,
            .y = bounds.y,
            .width = GRID_CELL_SIZE,
            .height = GRID_CELL_SIZE,
        },
        Vector2{
            GRID_CELL_SIZE * 0.1F,
            GRID_CELL_SIZE * 0.1F,
        });
    DrawPiece(slot.piece, icon);

    // Draw Quantity..
    auto posX = static_cast<int>(bounds.x + (bounds.width * 0.66F));
    auto posY = static_cast<int>(bounds.y + (bounds.height * 0.33F));
    std::string text = "x" + std::to_string(slot.count);
    DrawText(text.c_str(), posX, posY, 24, BLACK);
}
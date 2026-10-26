#pragma once

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define BOARD_ORIGIN_X 64.F
#define BOARD_ORIGIN_Y 64.F

#define GRID_CELL_SIZE 64.F
#define GRID_SLOT_WIDTH 128.F
#define GRID_SLOT_HEIGHT 64.F

#define INVENTORY_ORIGIN_X SCREEN_WIDTH - GRID_SLOT_WIDTH + 64.F
#define INVENTORY_ORIGIN_Y 64.F

#define RAY_STRENGTH_MAX 8

#include <raylib.h>

inline Rectangle Inset(Rectangle r, Vector2 pad)
{
    return { r.x + pad.x, r.y + pad.y, r.width - (2 * pad.x),
        r.height - (2 * pad.y) };
}
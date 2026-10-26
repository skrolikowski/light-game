#include <gtest/gtest.h>

#include "../src/Color.h"
#include "../src/Def.h"
#include "../src/Game.h"
#include "../src/Grid.h"
#include "../src/Piece.h"

TEST(Bleed, BleedOutOfBounds)
{
    Grid<Cell> board{ 2, 2, Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE } };
    Grid<LightCell> lc{ 2, 2, Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE } };

    Coords coords{ .row = 0, .col = 0 };
    LightCell &cell = *lc.GetItem(coords);
    cell.rays.push_back({ .out = AxisDirection::North });

    bool hasBleed = Game::Bleed(&board, &lc, coords);
    EXPECT_FALSE(hasBleed);
}

TEST(Bleed, NatrualBleed)
{
    Grid<Cell> board{ 2, 2, Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE } };
    Grid<LightCell> lc{ 2, 2, Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE } };

    // Start LightCell..
    Coords startCoords{ .row = 0, .col = 0 };
    LightCell &startCell = *lc.GetItem(startCoords);

    // End LightCell..
    Coords endCoords{ .row = 1, .col = 0 };
    LightCell &endCell = *lc.GetItem(endCoords);

    // Push Mock LightRay to Start LightCell..
    LightRay mockRay{
        .color = LightColor::Red,
        .strength = 5,
        .out = AxisDirection::South,
    };
    startCell.rays.push_back(mockRay);

    // Test..
    bool hasBleed = Game::Bleed(&board, &lc, startCoords);
    EXPECT_TRUE(hasBleed);

    ASSERT_EQ(endCell.rays.size(), 1);
    ASSERT_EQ(endCell.rays[0].color, LightColor::Red);
    ASSERT_EQ(endCell.rays[0].strength, mockRay.strength - 1);
    ASSERT_EQ(endCell.rays[0].in, AxisDirection::North);
}
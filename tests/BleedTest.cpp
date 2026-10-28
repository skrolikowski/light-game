#include <gtest/gtest.h>

#include "Board.h"
#include "Def.h"
#include "Game.h"
#include "Grid.h"

class BleedFixture : public ::testing::Test
{
  protected:
    Board board{
        2,
        2,
        Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE },
    };
    Grid<LightCell> lc{
        2,
        2,
        Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE },
    };
};

TEST_F(BleedFixture, OutOfBounds)
{
    Coords coords{ .row = 0, .col = 0 };
    LightCell &cell = *lc.GetItemAt(coords);
    cell.rays.push_back({ .out = AxisDirection::North });

    bool hasBleed = Game::Bleed(&board, &lc, coords);
    EXPECT_FALSE(hasBleed);
}

TEST_F(BleedFixture, NatrualBleed)
{
    // Start LightCell..
    Coords startCoords{ .row = 0, .col = 0 };
    LightCell &startCell = *lc.GetItemAt(startCoords);

    // End LightCell..
    Coords endCoords{ .row = 1, .col = 0 };
    LightCell &endCell = *lc.GetItemAt(endCoords);

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
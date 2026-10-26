#include <gtest/gtest.h>

#include "Def.h"
#include "Game.h"
#include "Grid.h"

class InputFixture : public ::testing::Test
{
  protected:
    Game game;
};

TEST_F(InputFixture, HoverOverBoardCellWithNoLevelLoaded)
{
    EXPECT_TRUE(game.GetBoard() == nullptr);
}

TEST_F(InputFixture, HoverOverBoardCellWithLevelLoaded)
{
    game.LoadLevel("res/levels/mock_unsolved.json");

    // Grid<Cell> *board = game.GetBoard();

    const Vector2 mousePos = {
        BOARD_ORIGIN_X + (GRID_CELL_SIZE / 2),
        BOARD_ORIGIN_Y + (GRID_CELL_SIZE / 2),
    };

    Cell *cell = game.GetBoard()->GetItemAt(mousePos);
    Coords coords = game.GetBoard()->GetCoordsAt(mousePos);

    EXPECT_TRUE(cell != nullptr);
    EXPECT_TRUE(coords == Coords{});
}

TEST_F(InputFixture, TestFalseResults)
{
    game.LoadLevel("res/levels/mock_unsolved.json");

    LightField field = Game::Simulate(game.GetBoard());
    LightFieldResult result = Game::CheckLightField(game.GetBoard(), field);

    EXPECT_TRUE(field.isStable);
    EXPECT_FALSE(result.isSolved());
}
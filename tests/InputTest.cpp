#include <gtest/gtest.h>

#include "Def.h"
#include "Game.h"
#include "Grid.h"
#include "Input.h"

class InputFixture : public ::testing::Test
{
  protected:
    Game game;
};
// TODO: test picking-up from the inventory or board
// TODO: test dropping on an empty cell
// TODO: test dropping on a full cell
// TODO: test dropping off the board (should snap back to original slot/cell)
// TODO: right-click rotate
// TODO: test that inventory counts stay correct

TEST_F(InputFixture, HoverOverBoardWithNoLevelLoaded)
{
    EXPECT_TRUE(game.GetBoard() == nullptr);
}

TEST_F(InputFixture, HoverOverBoardCellWithLevelLoaded)
{
    json level = R"({
        "title": "Mock Level",
        "grid": { "rows": 2, "cols": 2 },
        "pieces": [],
        "inventory": []
    })"_json;
    game.LoadLevel(level);

    Vector2 mockMousePosition = {
        BOARD_ORIGIN_X + (GRID_CELL_SIZE / 2),
        BOARD_ORIGIN_Y + (GRID_CELL_SIZE / 2),
    };

    MouseTarget target = game.GetMouseTargetAt(mockMousePosition);
    BoardHit *boardHit = std::get_if<BoardHit>(&target);

    ASSERT_NE(boardHit, nullptr);
    ASSERT_NE(boardHit->cell, nullptr);
    EXPECT_EQ(boardHit->coords, Coords{});
}

// TODO: test picking-up from the inventory or board
TEST_F(InputFixture, UserCannotPickUpNullPiece) {}
TEST_F(InputFixture, UserCannotPickUpNonMoveablePiece) {}

TEST_F(InputFixture, UserCanPickupMovablePiece)
{
    json level = R"({
        "title": "Mock Level",
        "grid": { "rows": 2, "cols": 2 },
        "pieces": [{"type":"Source}],
        "inventory": []
    })"_json;
    game.LoadLevel(level);

    Vector2 mockMousePosition = {
        BOARD_ORIGIN_X + (GRID_CELL_SIZE / 2),
        BOARD_ORIGIN_Y + (GRID_CELL_SIZE / 2),
    };
    bool changed = game.TrackMouseInput({
        .pos = mockMousePosition,
        .button = MouseButtonInput::LeftPressed,
    });
    ASSERT_TRUE(changed);

    // MouseTarget target = game.GetMouseTargetAt(mockMousePosition);
    // BoardHit *boardHit = std::get_if<BoardHit>(&target);
    // Drag drag = game.GetDrag();
    // EXPECT_EQ(std::holds_alternative<Wall>(const variant<Types...> &v))
    // EXPECT_EQ(drag.held, boardHit->cell->piece);
}

// TEST_F(InputFixture, TestFalseResults)
// {
//     game.LoadLevel("res/levels/mock_unsolved.json");

//     LightField field = Game::Simulate(game.GetBoard());
//     LightFieldResult result = Game::CheckLightField(game.GetBoard(), field);

//     EXPECT_TRUE(field.isStable);
//     EXPECT_FALSE(result.isSolved());
// }
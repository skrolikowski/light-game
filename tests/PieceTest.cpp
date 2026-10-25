#include <gtest/gtest.h>

#include "../src/Color.h"
#include "../src/Game.h"
#include "../src/Grid.h"
#include "../src/Piece.h"

class PieceFixture : public ::testing::Test
{
  protected:
    Grid<Cell> board;
    Grid<LightCell> lc;

    void SetUp() override
    {
        board = Grid<Cell>{ 2, 2 };
        lc = Grid<LightCell>{ 2, 2 };
    }

    void TearDown() override {}
};

// --------
// Transform Piece
// --------

TEST_F(PieceFixture, NullPieceTransform)
{
    // Start LightCell..
    Coords coords{ .row = 1, .col = 0 };
    LightCell &cell = lc.GetItem(coords);

    // Mock LightRay..
    LightRay ray = {
        .color = LightColor::Red,
        .strength = 4,
        .in = AxisDirection::North,
    };
    cell.rays.push_back(ray);

    Game::Transform(std::monostate{}, cell);

    ASSERT_EQ(cell.rays.size(), 1);
    EXPECT_EQ(cell.rays[0].out, AxisDirection::South);
}

TEST_F(PieceFixture, WallPieceTransform)
{
    // Start LightCell..
    Coords coords{ .row = 0, .col = 0 };
    LightCell &cell = lc.GetItem(coords);

    // Create and Wall Source Piece..
    Piece piece = Wall{};
    board.GetItem(coords).piece = piece;

    // Mock LightRay..
    LightRay ray = {
        .color = LightColor::Red,
        .strength = 4,
        .in = AxisDirection::North,
    };
    cell.rays.push_back(ray);

    Game::Transform(piece, cell);

    ASSERT_EQ(cell.rays.size(), 1);
    EXPECT_EQ(cell.rays[0].out, AxisDirection::None);
}

TEST_F(PieceFixture, SourcePieceTransform)
{
    // LightCell in question..
    Coords coords{ .row = 0, .col = 0 };
    LightCell cell = lc.GetItem(coords);

    // Create and place Source Piece..
    Piece piece = Source{
        .color = LightColor::Red,
        .out = AxisDirection::South,
        .strength = 4,
    };
    board.GetItem(coords).piece = piece;

    Game::Transform(piece, cell);

    EXPECT_EQ(cell.rays.size(), 1);
    EXPECT_EQ(cell.rays[0].color, LightColor::Red);
    EXPECT_EQ(cell.rays[0].strength, 4);
    EXPECT_EQ(cell.rays[0].out, AxisDirection::South);
}

TEST_F(PieceFixture, TargetPieceTransform)
{
    // Start LightCell..
    Coords coords{ .row = 0, .col = 0 };
    LightCell &cell = lc.GetItem(coords);

    // Create and Wall Source Piece..
    Piece piece = Target{ .color = LightColor::Red };
    board.GetItem(coords).piece = piece;

    // Mock LightRay..
    LightRay ray = {
        .color = LightColor::Red,
        .strength = 4,
        .in = AxisDirection::North,
    };
    cell.rays.push_back(ray);

    Game::Transform(piece, cell);

    ASSERT_EQ(cell.rays.size(), 1);
    EXPECT_EQ(cell.rays[0].out, AxisDirection::South);
}

TEST_F(PieceFixture, MirrorPieceTransformSouthToEast)
{
    Coords coords{ .row = 0, .col = 0 };
    LightCell &cell = lc.GetItem(coords);

    // Create and Wall Source Piece..
    Piece piece = Mirror{ .rot = MirrorRotation::ForwardSlash };
    board.GetItem(coords).piece = piece;

    // Mock LightRay..
    LightRay ray = {
        .color = LightColor::Red,
        .strength = 4,
        .in = AxisDirection::South,
    };
    cell.rays.push_back(ray);

    Game::Transform(piece, cell);

    ASSERT_EQ(cell.rays.size(), 1);
    EXPECT_EQ(cell.rays[0].out, AxisDirection::East);
    EXPECT_EQ(cell.rays[0].color, LightColor::Red);
    EXPECT_EQ(cell.rays[0].strength, 4);
}

// TEST_F(PieceFixture, MirrorPieceTransformSouthToWest) {}

// TEST_F(PieceFixture, MirrorPieceTransformNorthToWest) {}

// TEST_F(PieceFixture, MirrorPieceTransformNorthToWest) {}

TEST_F(PieceFixture, BoosterPieceTransform)
{
    Coords coords{ .row = 0, .col = 0 };
    LightCell &cell = lc.GetItem(coords);

    // Create and Wall Source Piece..
    Piece piece = Booster{ .strength = 3 };
    board.GetItem(coords).piece = piece;

    // Mock LightRay..
    LightRay ray = {
        .color = LightColor::Red,
        .strength = 2,
        .in = AxisDirection::North,
    };
    cell.rays.push_back(ray);

    Game::Transform(piece, cell);

    ASSERT_EQ(cell.rays.size(), 1);
    EXPECT_EQ(cell.rays[0].out, AxisDirection::South);
    EXPECT_EQ(cell.rays[0].color, LightColor::Red);
    EXPECT_EQ(cell.rays[0].strength, 5);
}

// TEST_F(PieceFixture, BoosterPieceTransformMaxStrength) {}

TEST_F(PieceFixture, InverterPieceTransformRedtoCyan)
{
    Coords coords{ .row = 0, .col = 0 };
    LightCell &cell = lc.GetItem(coords);

    // Create and Wall Source Piece..
    Piece piece = Inverter{};
    board.GetItem(coords).piece = piece;

    // Mock LightRay..
    LightRay ray = {
        .color = LightColor::Red,
        .strength = 5,
        .in = AxisDirection::North,
    };
    cell.rays.push_back(ray);

    Game::Transform(piece, cell);

    ASSERT_EQ(cell.rays.size(), 1);
    EXPECT_EQ(cell.rays[0].out, AxisDirection::South);
    EXPECT_EQ(cell.rays[0].color, LightColor::Cyan);
    EXPECT_EQ(cell.rays[0].strength, 5);
}

// ------------------------
// LightCell Bleeding Tests
// ------------------------

TEST_F(PieceFixture, BleedOutOfBounds)
{
    Coords coords{ .row = 0, .col = 0 };
    LightCell cell = lc.GetItem(coords);
    cell.rays.push_back({ .out = AxisDirection::North });

    bool hasBleed = Game::Bleed(&board, &lc, coords);
    EXPECT_FALSE(hasBleed);
}

TEST_F(PieceFixture, NatrualBleed)
{
    // Start LightCell..
    Coords startCoords{ .row = 0, .col = 0 };
    LightCell &startCell = lc.GetItem(startCoords);

    // End LightCell..
    Coords endCoords{ .row = 1, .col = 0 };
    LightCell &endCell = lc.GetItem(endCoords);

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
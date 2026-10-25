#include <gtest/gtest.h>

#include "../src/Color.h"
#include "../src/Def.h"
#include "../src/Game.h"
#include "../src/Grid.h"
#include "../src/Piece.h"

class PieceFixture : public ::testing::Test
{
  protected:
    Grid<Cell> board{
        2,
        2,
        Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE },
    };
    Grid<LightCell> lc{
        2,
        2,
        Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE },
    };

    static LightRay TransformOne(const Piece &piece, LightRay in)
    {
        LightCell cell;
        cell.rays.push_back(in);
        Game::Transform(piece, cell);
        EXPECT_EQ(cell.rays.size(), 1);
        return cell.rays.empty() ? LightRay{} : cell.rays[0];
    }
};

// --------
// Transform Piece
// --------

TEST_F(PieceFixture, NullPieceTransform)
{
    Piece piece = std::monostate{};
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::North };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
}

TEST_F(PieceFixture, WallPieceTransform)
{
    Piece piece = Wall{};
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::North };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::None);
}

TEST_F(PieceFixture, TargetPieceTransform)
{
    Piece piece = Target{ .color = LightColor::Red };
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::North };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
}

TEST_F(PieceFixture, MirrorPieceTransformSouthToEast)
{
    Piece piece = Mirror{ .rot = MirrorRotation::ForwardSlash };
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::South };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::East);
}

TEST_F(PieceFixture, MirrorPieceTransformSouthToWest)
{
    Piece piece = Mirror{ .rot = MirrorRotation::BackSlash };
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::South };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::West);
}

TEST_F(PieceFixture, MirrorPieceTransformNorthToWest)
{
    Piece piece = Mirror{ .rot = MirrorRotation::ForwardSlash };
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::North };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::West);
}

TEST_F(PieceFixture, MirrorPieceTransformNorthToEast)
{
    Piece piece = Mirror{ .rot = MirrorRotation::BackSlash };
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::North };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::East);
}

TEST_F(PieceFixture, BoosterPieceTransform)
{
    Piece piece = Booster{ .strength = 3 };
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::North };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
    EXPECT_EQ(out.color, LightColor::Red);
    EXPECT_EQ(out.strength, 5);
}

TEST_F(PieceFixture, BoosterPieceTransformMaxStrength)
{
    Piece piece = Booster{ .strength = 4 };
    LightRay ray{ .color = LightColor::Red, .strength = 4, .in = AxisDirection::North };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.strength, 5);
}

TEST_F(PieceFixture, InverterPieceTransformRedtoCyan)
{
    Piece piece = Inverter{};
    LightRay ray{ .color = LightColor::Red, .strength = 2, .in = AxisDirection::North };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
    EXPECT_EQ(out.color, LightColor::Cyan);
}

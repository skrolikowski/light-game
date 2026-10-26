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
        TransformPiece(piece, cell.rays);
        EXPECT_EQ(cell.rays.size(), 1);
        return cell.rays.empty() ? LightRay{} : cell.rays[0];
    }

    static LightRay TransformCombiner(
        const Combiner &combiner, const std::vector<LightRay> &rays)
    {
        LightRay merged{ .out = combiner.out };
        for (const auto &ray : rays)
        {
            if (ray.in == AxisDirection::None || ray.in == combiner.out)
                continue;
            merged.color = merged.color | ray.color;
            merged.strength = std::max(merged.strength, ray.strength);
            merged.strengthMax = merged.strength;
        }
        return merged;
    }
};

// -- Null Piece --------

TEST_F(PieceFixture, NullPieceTransform)
{
    Piece piece = std::monostate{};
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
}

// -- Wall Piece --------

TEST_F(PieceFixture, WallPieceTransform)
{
    Piece piece = Wall{};
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::None);
}

// -- Target Piece --------

TEST_F(PieceFixture, TargetPieceTransform)
{
    Piece piece = Target{ .color = LightColor::Red };
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::None);
}

// -- Mirror Piece --------

TEST_F(PieceFixture, MirrorPieceTransformSouthToEast)
{
    Piece piece = Mirror{ .rot = MirrorRotation::ForwardSlash };
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::South
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::East);
}

TEST_F(PieceFixture, MirrorPieceTransformSouthToWest)
{
    Piece piece = Mirror{ .rot = MirrorRotation::BackSlash };
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::South
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::West);
}

TEST_F(PieceFixture, MirrorPieceTransformNorthToWest)
{
    Piece piece = Mirror{ .rot = MirrorRotation::ForwardSlash };
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::West);
}

TEST_F(PieceFixture, MirrorPieceTransformNorthToEast)
{
    Piece piece = Mirror{ .rot = MirrorRotation::BackSlash };
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::East);
}

TEST_F(PieceFixture, MirrorPieceTransformRotation)
{
    Piece piece = Mirror{ .rot = MirrorRotation::ForwardSlash };
    RotatePiece(piece);

    EXPECT_EQ(std::get<Mirror>(piece).rot, MirrorRotation::BackSlash);
}

// -- Booster Piece --------

TEST_F(PieceFixture, BoosterPieceTransform)
{
    Piece piece = Booster{ .strength = 3 };
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
    EXPECT_EQ(out.color, LightColor::Red);
    EXPECT_EQ(out.strength, 5);
}

TEST_F(PieceFixture, BoosterPieceTransformMaxStrength)
{
    Piece piece = Booster{ .strength = 5 };
    LightRay ray{
        .color = LightColor::Red, .strength = 5, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.strength, RAY_STRENGTH_MAX);
}

// -- Inverter Piece --------

TEST_F(PieceFixture, InverterPieceTransformRedtoCyan)
{
    Piece piece = Inverter{};
    LightRay ray{
        .color = LightColor::Red, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
    EXPECT_EQ(out.color, LightColor::Cyan);
}

// -- Filter Piece --------

TEST_F(PieceFixture, FilterPieceTransformRedFromYellow)
{
    Piece piece = Filter{ .keep = LightColor::Red };
    LightRay ray{
        .color = LightColor::Yellow, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
    EXPECT_EQ(out.color, LightColor::Red);
}

TEST_F(PieceFixture, FilterPieceTransformRedFromCyan)
{
    Piece piece = Filter{ .keep = LightColor::Red };
    LightRay ray{
        .color = LightColor::Cyan, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
    EXPECT_EQ(out.color, LightColor::None);
}

// -- Tint Piece --------

TEST_F(PieceFixture, FilterPieceTransformRedAndGreenMakeYellow)
{
    Piece piece = Tint{ .add = LightColor::Red };
    LightRay ray{
        .color = LightColor::Green, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
    EXPECT_EQ(out.color, LightColor::Yellow);
}

TEST_F(PieceFixture, FilterPieceTransformBlueAndCyanMakeCyan)
{
    Piece piece = Tint{ .add = LightColor::Blue };
    LightRay ray{
        .color = LightColor::Cyan, .strength = 2, .in = AxisDirection::North
    };
    auto out = TransformOne(piece, ray);

    EXPECT_EQ(out.out, AxisDirection::South);
    EXPECT_EQ(out.color, LightColor::Cyan);
}

// -- Combiner Piece --------

TEST_F(PieceFixture, CombinerPieceTransform)
{
    auto piece = Combiner{ .out = AxisDirection::North };
    LightRay r1{
        .color = LightColor::Red, .strength = 3, .in = AxisDirection::West
    };
    LightRay r2{
        .color = LightColor::Blue, .strength = 2, .in = AxisDirection::West
    };
    auto out = TransformCombiner(piece, { r1, r2 });

    EXPECT_EQ(out.out, AxisDirection::North);
    EXPECT_EQ(out.color, LightColor::Magenta);
    EXPECT_EQ(out.strength, 3);
}

TEST_F(PieceFixture, CombinerPieceTransformRotationNorthToWest)
{
    Piece piece = Combiner{ .out = AxisDirection::North };
    RotatePiece(piece);

    EXPECT_EQ(std::get<Combiner>(piece).out, AxisDirection::West);
}

TEST_F(PieceFixture, CombinerPieceTransformRotationSouthToEast)
{
    Piece piece = Combiner{ .out = AxisDirection::South };
    RotatePiece(piece);

    EXPECT_EQ(std::get<Combiner>(piece).out, AxisDirection::East);
}
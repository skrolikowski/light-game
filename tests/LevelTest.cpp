#include <filesystem>
#include <gtest/gtest.h>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;
using namespace nlohmann::literals;

#include "Board.h"
#include "Game.h"
#include "Level.h"
#include "Loader.h"

struct LevelTestCase
{
    std::string path;
};

class LevelFixture : public ::testing::TestWithParam<LevelTestCase>
{
};

TEST_P(LevelFixture, MatchesSchema)
{
    json schema, level;
    LoadFileData("res/levels/level.schema.json", schema);
    LoadFileData(GetParam().path, level);

    json_validator validator;
    validator.set_root_schema(schema);
    EXPECT_NO_THROW(validator.validate(level));
}

TEST_P(LevelFixture, LoadsSuccessfully)
{
    // Retrieve the current data row
    const LevelTestCase &params = GetParam();

    json levelData = json::object({});
    EXPECT_NO_THROW(LoadFileData(params.path, levelData));
}

TEST_P(LevelFixture, NotSolvedOnStart)
{
    const LevelTestCase &params = GetParam();

    Game game;
    game.LoadLevel(params.path);

    Board *board = game.GetBoard();
    LightField field = Game::Simulate(board);
    LightFieldResult result = Game::CheckLightField(board, field);

    EXPECT_FALSE(result.isSolved());
}

TEST_P(LevelFixture, AllPiecesAddedToBoard)
{
    const LevelTestCase &params = GetParam();

    Game game;
    game.LoadLevel(params.path);
    Board *board = game.GetBoard();

    // Count number of pieces on board..
    int numPieces = board->GetNumPieces();

    json level;
    LoadFileData(GetParam().path, level);
    EXPECT_EQ(GetPieceCount(level), numPieces);
}

TEST_P(LevelFixture, AllPiecesAddedToInventory)
{
    const LevelTestCase &params = GetParam();

    Game game;
    game.LoadLevel(params.path);
    Grid<Slot> *inventory = game.GetBag();

    json level;
    LoadFileData(GetParam().path, level);

    inventory->ForEach(
        [&](const Slot &s, Coords coords)
        {
            std::size_t count =
                GetInventoryPieceCount(level, PieceToString(s.piece));
            EXPECT_EQ(s.count, count);
        });
}

// TODO: Brute Force is solvable check..
// TEST_P(LevelFixture, IsSolvable) {}

// --
// -- Level Loading Test Suite --------
static std::vector<LevelTestCase> AllLevels()
{
    std::vector<LevelTestCase> cases;
    for (const auto &entry : std::filesystem::directory_iterator("res/levels"))
        if (entry.path().extension() == ".json" &&
            entry.path().filename() != "level.schema.json")
            cases.push_back({ entry.path().string() });
    return cases;
}

INSTANTIATE_TEST_SUITE_P(
    LevelLoadingTests, LevelFixture, ::testing::ValuesIn(AllLevels()));

// ---
// --- EDGE CASE TESTING
// ---

class LevelEdgeCaseFixture : public ::testing::Test
{
  protected:
    Game game;

    void SetUp() override
    {
        json level = R"({
            "title": "Test",
            "grid": { "rows": 4, "cols": 4 },
            "pieces": [
                {
                    "type": "source",
                    "coords": { "row": 0, "col": 0 },
                    "color": "red",
                    "direction": "south",
                    "strength": 6
                },
                {
                    "type": "wall",
                    "coords": { "row": 0, "col": 0 }
                }
            ],
            "inventory": [
                {
                    "type": "mirror",
                    "rot": "/",
                    "count": 1
                },
                {
                    "type": "mirror",
                    "rot": "/",
                    "count": 1
                }
            ]
        })"_json;
        game.LoadLevel(level);
    }
};

TEST_F(LevelEdgeCaseFixture, TwoPiecesAtCoordsFirstWins)
{
    Board *board = game.GetBoard();
    Piece &p = board->GetPieceAt(Coords{});

    ASSERT_TRUE(std::holds_alternative<Source>(p));
    ASSERT_FALSE(std::holds_alternative<Wall>(p));
    // const Wall *piece = std::get_if<Wall>(&c00->piece);
    // EXPECT_TRUE(piece);
    // -- Load Schema..
    // json schema;
    // LoadFileData("res/levels/level.schema.json", schema);

    // // -- Load Validator..
    // json_validator validator;
    // validator.set_root_schema(schema);
    // EXPECT_NO_THROW(validator.validate(level));
}

TEST_F(LevelEdgeCaseFixture, DuplicateInventoryPiecesAreMerged)
{
    Grid<Slot> *bag = game.GetBag();
}

TEST(LevelLoadingEdgeCaseTests, OutOfBoundsPiecesAreIgnored) {}
TEST(LevelLoadingEdgeCaseTests, InvalidFormatMissingGrid) {}
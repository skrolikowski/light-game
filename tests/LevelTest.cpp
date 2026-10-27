#include <filesystem>
#include <gtest/gtest.h>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;

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
    LoadLevelData("res/levels/level.schema.json", schema);
    LoadLevelData(GetParam().path, level);

    json_validator validator;
    validator.set_root_schema(schema);
    EXPECT_NO_THROW(validator.validate(level));
}

TEST_P(LevelFixture, LoadsSuccessfully)
{
    // Retrieve the current data row
    const LevelTestCase &params = GetParam();

    json levelData = json::object({});
    EXPECT_NO_THROW(LoadLevelData(params.path, levelData));
}

TEST_P(LevelFixture, NotSolvedOnStart)
{
    const LevelTestCase &params = GetParam();

    Game game;
    game.LoadLevel(params.path);

    Grid<Cell> *board = game.GetBoard();
    LightField field = Game::Simulate(board);
    LightFieldResult result = Game::CheckLightField(board, field);

    EXPECT_FALSE(result.isSolved());
}

TEST_P(LevelFixture, AllPiecesAddedToBoard)
{
    const LevelTestCase &params = GetParam();

    Game game;
    game.LoadLevel(params.path);
    Grid<Cell> *board = game.GetBoard();

    // Count number of pieces on board..
    int numPieces = 0;
    board->ForEach([&](const Cell &cell, Coords coords)
        { numPieces += IsPieceEmpty(cell.piece) ? 0 : 1; });

    json level;
    LoadLevelData(GetParam().path, level);
    EXPECT_EQ(GetPieceCount(level), numPieces);
}

TEST_P(LevelFixture, AllPiecesAddedToInventory)
{
    // TODO: all pieces from level file added to inventory with correct count
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
#pragma once

#include <memory>
#include <string>

#include "Grid.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct Drag
{
    std::optional<Piece> held;
    std::optional<Coords> from;
    Slot *slot;
};

using LightRays = std::vector<LightRay>;
using CombinerOutputs = std::map<Coords, LightRay>;

struct LightCell
{
    std::vector<LightRay> rays;
};

struct LightField
{
    std::unique_ptr<Grid<LightCell>> grid;
    bool isStable = true;
};

struct LightFieldResult
{
    int numTargets = 0;
    int numTargetsLit = 0;
    bool isStable = true;

    [[nodiscard]] bool isSolved() const noexcept
    {
        return isStable && numTargets == numTargetsLit;
    }
};

class Game
{
  public:
    Game(int width, int height, const char *title);
    ~Game();

    void Run();

    static LightFieldResult CheckLightField(
        const Grid<Cell> *grid, const LightField &field);
    static LightField Propagate(
        Grid<Cell> *grid, const CombinerOutputs &outputs);
    static bool Bleed(
        Grid<Cell> *grid, Grid<LightCell> *lightGrid, Coords coords);
    static LightField Simulate(Grid<Cell> *grid);

    static void DrawInventorySlot(const Slot &slot, Rectangle bounds);

  private:
    void update(float dt);
    bool trackInput();
    void draw() const;
    void drawLightField() const;

    Drag drag_;
    LightField lightField_;
    LightFieldResult lightFieldResult_{};
    std::string levelTitle_;
    json levelData_ = json::object();
    std::unique_ptr<Grid<Cell>> board_;
    std::unique_ptr<Grid<Slot>> bag_;
};
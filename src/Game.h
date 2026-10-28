#pragma once

#include <memory>
#include <string>

#include "Board.h"
#include "Grid.h"
#include "Input.h"
#include "Level.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using LightRays = std::vector<LightRay>;
using CombinerOutputs = std::map<Coords, LightRay>;

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
        return isStable && numTargetsLit >= numTargets;
    }
};

class Game
{
  public:
    void LoadLevel(const std::string &path);
    void LoadLevel(const json &data);
    void Run();

    MouseTarget GetMouseTargetAt(Vector2 v);
    bool TrackMouseInput(MouseInput input);

    [[nodiscard]] const Drag &GetDrag() const noexcept { return drag_; }
    [[nodiscard]] Board *GetBoard() const noexcept { return board_.get(); }
    [[nodiscard]] Grid<Slot> *GetBag() const noexcept { return bag_.get(); }

    static LightFieldResult CheckLightField(
        Board *board, const LightField &field);
    static LightField Propagate(Board *board, const CombinerOutputs &outputs);
    static bool Bleed(Board *board, Grid<LightCell> *lightGrid, Coords coords);
    static LightField Simulate(Board *board);

    static void DrawInventorySlot(const Slot &slot, Rectangle bounds);

  private:
    void buildOutLevel();
    void update(float dt);
    void draw() const;
    void drawLightField() const;

    Drag drag_;
    Level level_;
    LightField lightField_;
    LightFieldResult lightFieldResult_{};
    std::unique_ptr<Board> board_;
    std::unique_ptr<Grid<Slot>> bag_;
};
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

class Game
{
  public:
    Game(int width, int height, const char *title);
    ~Game();

    void Run();

    static LightField Propagate(
        Grid<Cell> *grid, const CombinerOutputs &outputs);
    static bool Bleed(
        Grid<Cell> *grid, Grid<LightCell> *lightGrid, Coords coords);
    static void Transform(const Piece &piece, LightCell &cell);
    static LightField Simulate(Grid<Cell> *grid);

    static void DrawInventorySlot(const Slot &slot, Rectangle bounds);
    static void DrawPiece(const Piece &piece, Rectangle bounds);
    static void DrawLightRay(const LightRay &ray, Rectangle bounds);

  private:
    void update(float dt);
    bool trackInput();
    void draw() const;
    void drawLightField() const;

    Drag drag_;
    LightField lightField_;
    std::string levelTitle_;
    json levelData_ = json::object();
    std::unique_ptr<Grid<Cell>> board_;
    std::unique_ptr<Grid<Slot>> bag_;
    // std::vector<Slot> inventory_;
};
#pragma once

#include <memory>
#include <string>

#include "Grid.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Game
{
  public:
    Game(int width, int height, const char *title);
    ~Game();

    void Run();

    static LightField Propagate(Grid<Cell> *grid, const CombinerOutputs &outputs);
    static bool Bleed(Grid<Cell> *grid, Grid<LightCell> *lightGrid, Coords coords);
    static void Transform(const Piece &piece, LightCell &cell);
    static LightField Simulate(Grid<Cell> *grid);

  private:
    void update(float dt);
    void draw() const;
    void drawInventorySlot(const Slot &slot, Rectangle bounds) const;
    void drawPiece(const Piece &piece, Rectangle bounds) const;

    std::string levelTitle_;
    json levelData_ = json::object();
    std::unique_ptr<Grid<Cell>> board_;
    std::unique_ptr<Grid<Slot>> bag_;
    // std::vector<Slot> inventory_;
};
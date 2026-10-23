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

  private:
    void update(float dt);
    void draw() const;
    LightField simulate();

    static void Transform(const Piece &piece, LightCell &cell);

    std::string levelTitle_;
    json levelData_ = json::object();
    std::unique_ptr<Grid<Cell>> board_;
    std::vector<Slot> inventory_;
};
#pragma once

#include <memory>
#include <string>

#include "Grid.h"

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

    std::string title_;
    std::unique_ptr<Grid<Cell>> board_;
    std::vector<Slot> inventory_;
};
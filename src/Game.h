#pragma once

#include "Def.h"
#include <raylib.h>

class Game
{
  public:
    Game(int width, int height, const char *title);
    ~Game();

    void Run();

  private:
    void Update(float dt);
    void Draw() const;
};
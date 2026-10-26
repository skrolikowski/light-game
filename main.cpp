#include <raylib.h>

#include "Def.h"
#include "Game.h"

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Light Game");
    SetTargetFPS(60);

    Game game;
    game.LoadLevel("res/levels/level_1.json");
    game.Run();

    CloseWindow();
    return 0;
}

#include "Def.h"
#include "Game.h"

int main()
{
    Game game{ SCREEN_WIDTH, SCREEN_HEIGHT, "Light Game" };
    game.LoadLevel("res/levels/level_1.json");
    game.Run();
    return 0;
}
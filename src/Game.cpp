#include "Game.h"

Game::Game(int width, int height, const char *title)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);
    SetTargetFPS(60);
}

Game::~Game()
{
    CloseWindow();
}

void Game::Run()
{
    while (!WindowShouldClose())
    {
        Update(GetFrameTime());
        Draw();
    }
}

void Game::Update(float dt) {}

void Game::Draw() const
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawFPS(SCREEN_WIDTH - 90, 10);

    EndDrawing();
}
#include <raylib.h>

#include "Game.h"
#include "Piece.h"

Game::Game(int width, int height, const char *title)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);
    SetTargetFPS(60);

    board_ = std::make_unique<Grid<Cell>>(Grid<Cell>{ { .x = 64.F, .y = 64.F }, 4, 4 });
    simulate();
}

LightField Game::simulate()
{
    int cols = board_->NumCols();
    int rows = board_->NumRows();

    bool isReady = false;
    LightField f{ .grid = { rows, cols } };

    do
    {
        int lightsSpreading = 0;

        f.grid.ForEach(
            [&](LightCell lc, Coords coords)
            {
                // Ray meets Piece (maybe)..
                Piece &piece = board_->GetItem(coords).piece;
                Transform(piece, lc);

                // Spread LightRays if possible..
                for (LightRay &ray : lc.rays)
                {
                    if (!ray.hasSpread && ray.strength > 0)
                    {
                        const Coords nextCoords = Translate(ray.out, coords);
                        if (Grid<Cell>::InBounds(board_.get(), nextCoords))
                        {
                            LightRay nextRay{
                                .color = ray.color,
                                .strength = ray.strength - 1,
                                .in = ray.out,
                            };
                            f.grid.GetItem(nextCoords).rays.push_back(nextRay);

                            lightsSpreading += 1;
                        }

                        // --
                        ray.hasSpread = true;
                    }
                }
            });

        isReady = lightsSpreading == 0;
    } while (!isReady);
}

void Game::Transform(const Piece &piece, LightCell &cell)
{
    std::visit(
        Overload{
            [&](std::monostate)
            {
                for (LightRay &r : cell.rays)
                    r.out = r.in;
            },
            [&](const Wall &) { /* absorbs light */ },
            [&](const Source &o)
            {
                LightRay source = {
                    .color = o.color,
                    .strength = o.strength,
                    .out = o.out,
                };
                cell.rays.push_back(source);
            },
            [&](const Target &)
            {
                for (LightRay &r : cell.rays)
                    r.out = r.in;
            },
            [&](const Mirror &o)
            {
                for (LightRay &r : cell.rays)
                    r.out = RotateDirection(r.in, o.rot);
            },
            [&](const Booster &o)
            {
                for (LightRay &r : cell.rays)
                    r.strength += o.strength;
            },
            [&](const Inverter &)
            {
                for (LightRay &r : cell.rays)
                    r.color = invert(r.color);
            },
            [&](const Filter &o)
            {
                for (LightRay &r : cell.rays)
                    r.color = r.color & o.keep;
            },
            [&](const Tint &o)
            {
                for (LightRay &r : cell.rays)
                    r.color = r.color | o.add;
            },
            [&](const Combiner &o)
            {
                LightRay merged;
                for (const LightRay &r : cell.rays)
                {
                    merged.color = merged.color | r.color;
                    merged.strength = std::max(merged.strength, r.strength);
                }
                merged.out = o.out;
                cell.rays.push_back(merged);
            },
        },
        piece);
}

// LightField Game::simulate()
// {
//     int cols = board_->NumCols();
//     int rows = board_->NumRows();
//     int dirCount = static_cast<int>(AxisDirection::Count);

//     LightField f{};
//     f.in = Grid<LightRays>{ rows, cols };
//     f.out = Grid<LightRays>{ rows, cols };

//     const int max_steps = (cols * rows * dirCount) + 2;
//     for (int i = 1; i <= max_steps; ++i)
//     {
//         for (int x = 0; x < cols; ++x)
//         {
//             for (int y = 0; y < rows; ++y)
//             {
//                 Coords coords = { .row = y, .col = x };
//                 Cell cell = board_->GetItem(coords);
//                 Piece &piece = cell.piece;
//                 transform(piece, f.in.GetItem(coords), f.out.GetItem(coords));
//             }
//         }

//         Grid<LightRays> next{ rows, cols };
//         for (int x = 0; x < cols; ++x)
//         {
//             for (int y = 0; y < rows; ++y)
//             {
//                 for (int d = 0; d < dirCount; ++d)
//                 {
//                     Coords coords = { .row = y, .col = x };
//                     auto dir = static_cast<AxisDirection>(d);
//                     LightRay ray = f.out.GetItem(coords)[static_cast<std::size_t>(d)];
//                     const Coords nextPos = Translate(dir, Coords{ .row = y, .col = x });

//                     if (ray.isLit() && ray.strength > 0 &&
//                         Grid<Cell>::InBounds(board_.get(), nextPos))
//                     {
//                         ray.strength--;
//                         next.GetItem(nextPos)[static_cast<std::size_t>(d)] = ray;
//                     }
//                 }
//             }
//         }

//         if (next == f.in)
//             return f;
//         f.in = std::move(next);
//     }
//     f.isStable = false;
//     return f;
// }

// void Game::transform(const Piece &piece, const LightRays &in, LightRays &out)
// {
//     auto each = [&](auto fn)
//     {
//         auto dirCount = static_cast<std::size_t>(AxisDirection::Count);
//         for (std::size_t dir = 0; dir < dirCount; ++dir)
//             if (in[dir].isLit())
//                 out[dir] = fn(in[dir]);
//     };

//     std::visit(
//         Overload{
//             [&](std::monostate) { out = in; },
//             [&](const Wall &) { /* absorbs light */ },
//             [&](const Source &o)
//             {
//                 LightRay source = { .color = o.color, .strength = o.strength };
//                 out[static_cast<std::size_t>(o.out)] = source;
//             },
//             [&](const Target &) { out = in; },
//             [&](const Mirror &)
//             {
//                 each(
//                     [&](LightRay r)
//                     {
//                         // TODO: redirects light 90deg
//                         return LightRay{
//                             .color = r.color,
//                             .strength = r.strength,
//                         };
//                     });
//             },
//             [&](const Booster &o)
//             {
//                 each(
//                     [&](LightRay r)
//                     {
//                         return LightRay{
//                             .color = r.color,
//                             .strength = r.strength + o.strength,
//                         };
//                     });
//             },
//             [&](const Inverter &)
//             {
//                 each(
//                     [&](LightRay r)
//                     {
//                         return LightRay{
//                             .color = invert(r.color),
//                             .strength = r.strength,
//                         };
//                     });
//             },
//             [&](const Filter &o)
//             {
//                 each(
//                     [&](LightRay r)
//                     {
//                         return LightRay{
//                             .color = r.color & o.keep,
//                             .strength = r.strength,
//                         };
//                     });
//             },
//             [&](const Tint &o)
//             {
//                 each(
//                     [&](LightRay r)
//                     {
//                         return LightRay{
//                             .color = r.color | o.add,
//                             .strength = r.strength,
//                         };
//                     });
//             },
//             [&](const Combiner &o)
//             {
//                 LightRay merged;
//                 for (const LightRay &r : in)
//                 {
//                     if (!r.isLit())
//                         continue;
//                     merged.color = merged.color | r.color;
//                     merged.strength = std::max(merged.strength, r.strength);
//                 }
//                 out[static_cast<std::size_t>(o.out)] = merged;
//             },
//         },
//         piece);
// }

Game::~Game()
{
    CloseWindow();
}

void Game::Run()
{
    while (!WindowShouldClose())
    {
        update(GetFrameTime());
        draw();
    }
}

void Game::update(float /*dt*/) {}

void Game::draw() const
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    board_->Draw();

    DrawFPS(SCREEN_WIDTH - 90, 10);
    EndDrawing();

    DrawText("raylib 3D grid example", 10, 10, 20, DARKGRAY);
}
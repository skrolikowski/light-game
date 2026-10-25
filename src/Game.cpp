#include <raylib.h>

#include <algorithm>

#include "Game.h"
#include "Loader.h"
#include "Piece.h"

Game::Game(int width, int height, const char *title)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title);
    SetTargetFPS(60);

    // load level
    LoadLevel("res/levels/level_1.json", levelData_);

    levelTitle_ = levelData_.value("title", "Default Title");
    int rows = levelData_.at("grid").value("rows", 0);
    int cols = levelData_.at("grid").value("cols", 0);
    Vector2 origin{ 64.F, 64.F };

    // Create game board..
    board_ = std::make_unique<Grid<Cell>>(Grid<Cell>{ origin, rows, cols });

    // Place pieces on board_..
    const json &pieces = levelData_.at("pieces");
    const int numPieces = static_cast<int>(pieces.size());
    for (int i = 0; i < numPieces; ++i)
    {
        const json &pieceData = pieces.at(static_cast<unsigned long>(i));
        Piece piece = ReadPiece(pieceData);

        if (pieceData.contains("coords"))
        {
            int row = pieceData.at("coords").value("row", 0);
            int col = pieceData.at("coords").value("col", 0);

            // place on board_
            Coords coords = { .row = row, .col = col };
            board_->GetItem(coords).piece = piece;
        }
    }

    // TODO: only fire on "dirty"..
    lightField_ = Simulate(board_.get());

    // -- Inventory ----
    const json &inventory = levelData_.at("inventory");
    const int numInventory = static_cast<int>(inventory.size());
    // Create slot bag..
    Vector2 bagOrigin{ SCREEN_WIDTH - (GRID_SLOT_WIDTH + 64.F), 64.F };
    bag_ = std::make_unique<Grid<Slot>>(Grid<Slot>{ bagOrigin, numInventory, 1 });
    int bagIndex = 0;
    bag_->ForEach(
        [&](Slot &s, Coords coords)
        {
            const json &invData = inventory.at(static_cast<unsigned long>(bagIndex));
            Piece piece = ReadPiece(invData);
            int pieceCount = invData.value("count", 1);
            s = Slot{ .piece = piece, .count = pieceCount };
            bagIndex++;
        });
    // for (int i = 0; i < numInventory; ++i)
    // {
    //     bag_->GetItem(Coords coords)
    //     const json &invData = inventory.at(static_cast<unsigned long>(i));
    //     Piece piece = ReadPiece(invData);
    //     int pieceCount = invData.value("count", 1);
    //     Slot slot{ .piece = piece, .count = pieceCount };
    //     inventory_.push_back(slot);
    // }
}

bool Game::Bleed(Grid<Cell> *grid, Grid<LightCell> *lightGrid, Coords coords)
{
    LightCell &cell = lightGrid->GetItem(coords);
    std::size_t bleedCount = 0;

    for (LightRay &ray : cell.rays)
    {
        if (!ray.hasSpread && ray.strength > 0)
        {
            // ..ray has ended..
            if (ray.out == AxisDirection::None)
            {
                ray.hasSpread = true;
                continue;
            }

            // ..bleed to next cell..
            const Coords nextCoords = Translate(ray.out, coords);
            if (Grid<Cell>::InBounds(grid, nextCoords))
            {
                LightRay nextRay{
                    .color = ray.color,
                    .strength = ray.strength - 1,
                    .in = InvertDirection(ray.out),
                };
                lightGrid->GetItem(nextCoords).rays.push_back(nextRay);
                bleedCount += 1;
            }

            // --
            ray.hasSpread = true;
        }
    }

    return bleedCount > 0;
}

LightField Game::Propagate(Grid<Cell> *grid, const CombinerOutputs &outputs)
{
    LightField field{
        .grid = std::make_unique<Grid<LightCell>>(Grid<LightCell>{
            grid->NumRows(),
            grid->NumCols(),
        }),
    };

    // 1) Create Source + Combiner Lights..
    field.grid->ForEach(
        [&](LightCell &lc, Coords coords)
        {
            Piece &piece = grid->GetItem(coords).piece;

            if (std::holds_alternative<Source>(piece))
            {
                Transform(piece, lc);
                Bleed(grid, field.grid.get(), coords);
            }
            else if (std::holds_alternative<Combiner>(piece))
            {
                // Push merged as new source..
                if (auto it = outputs.find(coords); it != outputs.end())
                {
                    lc.rays.push_back(it->second);
                    Bleed(grid, field.grid.get(), coords);
                }
            }
        });

    // 2) Propogate light..
    int lightsPropagating;
    do
    {
        lightsPropagating = 0; // reset
        field.grid->ForEach(
            [&](LightCell &lc, Coords coords)
            {
                // Ray meets Piece (maybe)..
                Piece &piece = grid->GetItem(coords).piece;

                // Skip Source Pieces..
                const auto *sourcePiece = std::get_if<Source>(&piece);
                if (sourcePiece != nullptr)
                    return;

                // Skip Combiner Pieces..
                const auto *combinerPiece = std::get_if<Combiner>(&piece);
                if (combinerPiece != nullptr)
                    return;

                // Hook in => out (if possible)..
                Transform(piece, lc);

                // Bleed to next coords..
                if (Bleed(grid, field.grid.get(), coords))
                    lightsPropagating += 1;
            });

    } while (lightsPropagating > 0);

    return field;
}

LightField Game::Simulate(Grid<Cell> *grid)
{
    LightField field;
    CombinerOutputs outputs;

    for (int i = 0; i < 2; ++i)
    {
        // 1) Handle Pieces (- Combiner)..
        field = Propagate(grid, outputs);

        // 3) Handle Combiner Pieces..
        CombinerOutputs next;
        field.grid->ForEach(
            [&](LightCell &lc, Coords coords)
            {
                Piece &piece = grid->GetItem(coords).piece;
                const auto *combinerPiece = std::get_if<Combiner>(&piece);
                if (combinerPiece != nullptr)
                {
                    LightRay merged{ .out = combinerPiece->out };

                    for (const LightRay &r : lc.rays)
                    {
                        if (r.in == AxisDirection::None || r.in == combinerPiece->out)
                            continue;
                        merged.color = merged.color | r.color;
                        merged.strength = std::max(merged.strength, r.strength);
                    }
                    if (merged.strength > 0)
                        next[coords] = merged;
                }
            });

        if (next == outputs)
            return field;

        outputs = std::move(next);
    }

    field.isStable = false;
    return field;
}

void Game::Transform(const Piece &piece, LightCell &cell)
{
    auto each = [&](auto fn)
    {
        for (LightRay &r : cell.rays)
        {
            if (r.hasSpread || r.strength == 0)
                continue;
            fn(r);
        }
    };

    std::visit(
        Overload{
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
            [&](const Mirror &o)
            { each([&](LightRay &r) { r.out = RotateDirection(r.in, o.rot); }); },
            [&](const Booster &o)
            {
                each(
                    [&](LightRay &r)
                    {
                        r.strength += o.strength;
                        r.out = InvertDirection(r.in);
                    });
            },
            [&](const Inverter &)
            {
                each(
                    [](LightRay &r)
                    {
                        r.color = invert(r.color);
                        r.out = InvertDirection(r.in);
                    });
            },
            [&](const Filter &o)
            {
                each(
                    [&](LightRay &r)
                    {
                        r.color = r.color & o.keep;
                        r.out = InvertDirection(r.in);
                    });
            },
            [&](const Tint &o)
            {
                each(
                    [&](LightRay &r)
                    {
                        r.color = r.color | o.add;
                        r.out = InvertDirection(r.in);
                    });
            },
            [&](auto &)
            {
                // Pass-thru..
                each([](LightRay &r) { r.out = InvertDirection(r.in); });
            },
        },
        piece);
}

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

void Game::drawInventorySlot(const Slot &slot, Rectangle bounds) const
{
    DrawRectangleRoundedLinesEx(bounds, 0.2F, 20, 1.F, BLACK);
    DrawRectangleRounded(bounds, 0.2F, 20, (slot.count > 0 ? WHITE : DARKGRAY));

    // Draw Slot Piece
    Rectangle icon = Inset(
        Rectangle{
            .x = bounds.x,
            .y = bounds.y,
            .width = GRID_CELL_SIZE,
            .height = GRID_CELL_SIZE,
        },
        GRID_CELL_SIZE * 0.1F);
    drawPiece(slot.piece, icon);

    // Draw Quantity..
    auto posX = static_cast<int>(bounds.x + (bounds.width * 0.66F));
    auto posY = static_cast<int>(bounds.y + (bounds.height * 0.33F));
    std::string text = "x" + std::to_string(slot.count);
    DrawText(text.c_str(), posX, posY, 24, BLACK);
}

void Game::drawPiece(const Piece &piece, Rectangle bounds) const
{
    const Rectangle r = Inset(bounds, GRID_CELL_SIZE * 0.1F);
    const Vector2 center{ r.x + (r.width / 2), r.y + (r.height / 2) };

    std::visit(
        Overload{
            [](std::monostate) { /*Draw nothing*/ },
            [&](const Wall &)
            {
                DrawRectangleRounded(r, 0.2F, 20, BLACK);
                // DrawRectangleRec(r, 5,BLACK);
            },
            [&](const Source &o) 
            {
                float wedge = 30.F;
                float radMin = r.width / 4.F;
                float radMax = r.width / 2.F;
                float rad = radMin + ((radMax-radMin) * static_cast<float>(o.strength) / SOURCE_STRENGTH_MAX);
                const float facing = DirectionToAngle(o.out);

                DrawCircleSector(center, rad, facing - wedge, facing + wedge, 20,
                    ToRaylibColor(o.color));
            },
            [&](const Target &o)
            {
                float radMin = r.width / 4.F;
                float radMax = r.width / 3.F;
                DrawRing(center, radMin, radMax, 0, 360, 32, ToRaylibColor(o.color));
            },
            [&](const Mirror &o)
            {
                DrawRectangleRoundedLinesEx(r, 0.2F, 20, 4.F, BLACK);
                
                const bool fwd = o.rot == MirrorRotation::ForwardSlash;
                Vector2 a{ r.x, fwd ? r.y + r.height : r.y };
                Vector2 b{ r.x + r.width, fwd ? r.y : r.y + r.height };
                DrawLineEx(a, b, 4.F, BLACK);
            },
            [&](const Booster &o)
            {
                // if (o.out == AxisDirection::None)
                // {
                //     DrawCircleV(center, rad, ToRaylibColor(o.color));
                //     return;
                // }
                DrawRectangleRoundedLinesEx(r, 0.2F, 20, 4.F, BLACK);
                std::string s = "+" + std::to_string(o.strength);
                DrawText(s.c_str(), center.x, center.y, 10, WHITE);
            },
            [&](const Inverter &)
            {
                
            },
            [&](const Filter &o)
            {
                DrawRing(center, r.width / 4, r.width / 3, 0, 360, 32, ToRaylibColor(o.keep));
            },
            [&](const Tint &o)
            {
                DrawCircleV(center, r.width / 3, ToRaylibColor(o.add));
            },
            [&](const Combiner &o)
            {
                //TODO: Rounded Rectangle Border
                //TODO: Tiny o.color Triangle pointing in out direction 
                // Triangle pointing East, relative to center; rotated to face o.out
                const float h = r.width / 2.F;
                const float angle = DirectionToAngle(o.out) * DEG2RAD;
                const Vector2 tip = Vector2Add(center, Vector2Rotate({ h, 0.F }, angle));
                const Vector2 baseA = Vector2Add(center, Vector2Rotate({ -h, -h }, angle));
                const Vector2 baseB = Vector2Add(center, Vector2Rotate({ -h, h }, angle));
                DrawTriangle(tip, baseA, baseB, BLACK); // raylib needs counter-clockwise order
            },
            // [](auto&) {},
        },
        piece);
}

void Game::drawLightRay(const LightRay &ray, Rectangle bounds) const
{
    const Vector2 center{ bounds.x + (bounds.width / 2), bounds.y + (bounds.height / 2) };
    const Vector2 north{ center.x, bounds.y };
    const Vector2 south{ center.x, bounds.y + bounds.height };
    const Vector2 east{ bounds.x + bounds.width, center.y };
    const Vector2 west{ bounds.x, center.y };
    const Color rayColor = ToRaylibColor(ray.color);
    const int glowThick = 16;
    const int rayThick = 5;
    const float strenghtFactor = static_cast<float>(ray.strength) / SOURCE_STRENGTH_MAX;

    // draw in
    switch (ray.in)
    {
        case AxisDirection::North:
            DrawLineEx(north, center, glowThick, Fade(rayColor, 0.18F * strenghtFactor));
            DrawLineEx(north, center, rayThick, Fade(rayColor, strenghtFactor));
            break;
        case AxisDirection::South:
            DrawLineEx(south, center, glowThick, Fade(rayColor, 0.18F * strenghtFactor));
            DrawLineEx(south, center, rayThick, Fade(rayColor, strenghtFactor));
            break;
        case AxisDirection::East:
            DrawLineEx(east, center, glowThick, Fade(rayColor, 0.18F * strenghtFactor));
            DrawLineEx(east, center, rayThick, Fade(rayColor, strenghtFactor));
            break;
        case AxisDirection::West:
            DrawLineEx(west, center, glowThick, Fade(rayColor, 0.18F * strenghtFactor));
            DrawLineEx(west, center, rayThick, Fade(rayColor, strenghtFactor));
            break;
        default:
            break;
    }

    // draw out
    switch (ray.out)
    {
        case AxisDirection::North:
            DrawLineEx(center, north, glowThick, Fade(rayColor, 0.18F * strenghtFactor));
            DrawLineEx(center, north, rayThick, Fade(rayColor, strenghtFactor));
            break;
        case AxisDirection::South:
            DrawLineEx(center, south, glowThick, Fade(rayColor, 0.18F * strenghtFactor));
            DrawLineEx(center, south, rayThick, Fade(rayColor, strenghtFactor));
            break;
        case AxisDirection::East:
            DrawLineEx(center, east, glowThick, Fade(rayColor, 0.18F * strenghtFactor));
            DrawLineEx(center, east, rayThick, Fade(rayColor, strenghtFactor));
            break;
        case AxisDirection::West:
            DrawLineEx(center, west, glowThick, Fade(rayColor, 0.18F * strenghtFactor));
            DrawLineEx(center, west, rayThick, Fade(rayColor, strenghtFactor));
            break;
        default:
            break;
    }
}

void Game::draw() const
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Draw board_ w/ Pieces..
    board_->ForEach(
        [&](const Cell &cell, Coords coords)
        {
            Vector2 topLeft = board_->CellTopLeft(coords);
            Rectangle bounds = {
                .x = topLeft.x,
                .y = topLeft.y,
                .width = GRID_CELL_SIZE,
                .height = GRID_CELL_SIZE,
            };

            DrawRectangleLinesEx(bounds, 1.F, LIGHTGRAY);
            drawPiece(cell.piece, bounds);
        });

    lightField_.grid->ForEach(
        [&](const LightCell &cell, Coords coords)
        {
            Vector2 topLeft = board_->CellTopLeft(coords);
            Rectangle bounds = {
                .x = topLeft.x,
                .y = topLeft.y,
                .width = GRID_CELL_SIZE,
                .height = GRID_CELL_SIZE,
            };
            for (const LightRay &ray : cell.rays)
            {
                drawLightRay(ray, bounds);
            }
        });

    // Draw bag_ w/ Slots..
    bag_->ForEach(
        [&](const Slot &s, Coords coords)
        {
            Vector2 topLeft = bag_->CellTopLeft(coords);
            Rectangle bounds = {
                .x = topLeft.x,
                .y = topLeft.y,
                .width = GRID_SLOT_WIDTH,
                .height = GRID_SLOT_HEIGHT,
            };
            drawInventorySlot(s, bounds);
        });

    DrawFPS(SCREEN_WIDTH - 90, 10);
    EndDrawing();
}
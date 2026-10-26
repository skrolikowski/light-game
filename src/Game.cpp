#include <raylib.h>

#include <algorithm>

#include "Def.h"
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
    board_ = std::make_unique<Grid<Cell>>(Grid<Cell>{
        origin,
        rows,
        cols,
        Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE },
    });

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
            board_->GetItem(coords)->piece = piece;
        }
    }

    lightField_ = Simulate(board_.get());

    // -- Inventory ----
    const json &inventory = levelData_.at("inventory");
    const int numInventory = static_cast<int>(inventory.size());
    // Create slot bag..
    Vector2 bagOrigin{ SCREEN_WIDTH - (GRID_SLOT_WIDTH + 64.F), 64.F };
    bag_ = std::make_unique<Grid<Slot>>(Grid<Slot>{
        bagOrigin,
        numInventory,
        1,
        Vector2{ GRID_SLOT_WIDTH, GRID_SLOT_HEIGHT },
    });

    int bagIndex = 0;
    bag_->ForEach(
        [&](Slot &s, Coords)
        {
            const json &invData =
                inventory.at(static_cast<unsigned long>(bagIndex));
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

Game::~Game()
{
    CloseWindow();
}

bool Game::Bleed(Grid<Cell> *grid, Grid<LightCell> *lightGrid, Coords coords)
{
    LightCell *cell = lightGrid->GetItem(coords);
    if (cell == nullptr)
        return false;
    std::size_t bleedCount = 0;

    for (LightRay &ray : cell->rays)
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
                lightGrid->GetItem(nextCoords)->rays.push_back(nextRay);
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
            Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE },
        }),
    };

    // 1) Create Source + Combiner Lights..
    field.grid->ForEach(
        [&](LightCell &lc, Coords coords)
        {
            Piece &piece = grid->GetItem(coords)->piece;

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
                Piece &piece = grid->GetItem(coords)->piece;

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
                Piece &piece = grid->GetItem(coords)->piece;
                const auto *combinerPiece = std::get_if<Combiner>(&piece);
                if (combinerPiece != nullptr)
                {
                    LightRay merged{ .out = combinerPiece->out };

                    for (const LightRay &r : lc.rays)
                    {
                        if (r.in == AxisDirection::None ||
                            r.in == combinerPiece->out)
                            continue;
                        merged.color = merged.color | r.color;
                        merged.strength = std::max(merged.strength, r.strength);
                        merged.strengthMax = merged.strength;
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
            [](const Wall &) { /* absorbs light */ },
            [&](const Source &o)
            {
                LightRay source = {
                    .color = o.color,
                    .strength = o.strength,
                    .strengthMax = o.strength,
                    .out = o.out,
                };
                cell.rays.push_back(source);
            },
            [](const Target &) { /* absorbs light */ },
            [&](const Mirror &o)
            {
                each([&](LightRay &r)
                    { r.out = RotateMirrorDirection(r.in, o.rot); });
            },
            [&](const Booster &o)
            {
                each(
                    [&](LightRay &r)
                    {
                        r.strength =
                            std::min(r.strength + o.strength, RAY_STRENGTH_MAX);
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

void Game::Run()
{
    while (!WindowShouldClose())
    {
        update(GetFrameTime());
        draw();
    }
}

void Game::update(float /*dt*/)
{
    if (trackInput())
    {
        // Update status..
        lightField_ = Simulate(board_.get());
    }
}

bool Game::trackInput()
{
    bool changed = false;

    // --
    const Vector2 m = GetMousePosition();
    const Coords coords = bag_->GetCoordsAt(m);
    Cell *cell = board_->GetItemAt(m);
    Slot *slot = bag_->GetItemAt(m);

    // LEFT MOUSE PRESS - Pick up
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !drag_.held)
    {
        if (slot != nullptr && slot->count > 0)
        {
            drag_.held = slot->piece;
            drag_.from = std::nullopt;
            drag_.slot = slot;

            slot->count--;
        }
        else if (cell != nullptr && cell->isMovable())
        {
            drag_.held = std::exchange(cell->piece, std::monostate{});
            drag_.from = coords;
            drag_.slot = cell->slot;
            changed = true;
        }
    }

    // RIGHT MOUSE PRESS - Rotate Piece
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        if (drag_.held.has_value())
        {
            RotatePiece(*drag_.held);
        }
        else if (cell != nullptr && cell->isMovable())
        {
            RotatePiece(cell->piece);
            changed = true;
        }
    }

    // LEFT MOUSE RELEASE - Place
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && drag_.held)
    {
        if (cell != nullptr && IsPieceEmpty(cell->piece))
        {
            // drop on board_
            cell->piece = *drag_.held;
            cell->slot = drag_.slot;
            changed = true;
        }
        else if (drag_.from.has_value() && drag_.slot == nullptr)
        {
            // snap back to board..
            board_->GetItem(*drag_.from)->piece = *drag_.held;
            changed = true;
        }
        else
        {
            // snap back to inventory..
            ++drag_.slot->count;
        }
        drag_ = {};
    }

    return changed;
}

void Game::DrawInventorySlot(const Slot &slot, Rectangle bounds)
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
        Vector2{
            GRID_CELL_SIZE * 0.1F,
            GRID_CELL_SIZE * 0.1F,
        });
    DrawPiece(slot.piece, icon);

    // Draw Quantity..
    auto posX = static_cast<int>(bounds.x + (bounds.width * 0.66F));
    auto posY = static_cast<int>(bounds.y + (bounds.height * 0.33F));
    std::string text = "x" + std::to_string(slot.count);
    DrawText(text.c_str(), posX, posY, 24, BLACK);
}

void Game::DrawPiece(const Piece &piece, Rectangle bounds)
{
    const Rectangle r =
        Inset(bounds, Vector2{ GRID_CELL_SIZE * 0.1F, GRID_CELL_SIZE * 0.1F });
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
        float rad =
            radMin + ((radMax - radMin) * static_cast<float>(o.strength) /
                         RAY_STRENGTH_MAX);
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
        DrawRing(center, r.width / 4, r.width / 3, 0, 360, 32,
            ToRaylibColor(o.keep));
            },
            [&](const Tint &o)
            {
        DrawCircleV(center, r.width / 3, ToRaylibColor(o.add));
            },
            [&](const Combiner &o)
            {
        // TODO: Rounded Rectangle Border
        // TODO: Tiny o.color Triangle pointing in out direction
        //  Triangle pointing East, relative to center; rotated to face o.out
        const float h = r.width / 2.F;
        const float angle = DirectionToAngle(o.out) * DEG2RAD;
        const Vector2 tip =
            Vector2Add(center, Vector2Rotate({ h, 0.F }, angle));
        const Vector2 baseA =
            Vector2Add(center, Vector2Rotate({ -h, -h }, angle));
        const Vector2 baseB =
            Vector2Add(center, Vector2Rotate({ -h, h }, angle));
        DrawTriangle(
            tip, baseA, baseB, BLACK); // raylib needs counter-clockwise order
            },
            // [](auto&) {},
        },
        piece);
}

void Game::DrawLightRay(const LightRay &ray, Rectangle bounds)
{
    const Vector2 center{ bounds.x + (bounds.width / 2),
        bounds.y + (bounds.height / 2) };

    // Edge midpoint for a direction (center if None)..
    auto edge = [&](AxisDirection dir) -> Vector2
    {
        switch (dir)
        {
            case AxisDirection::North:
                return { center.x, bounds.y };
            case AxisDirection::South:
                return { center.x, bounds.y + bounds.height };
            case AxisDirection::East:
                return { bounds.x + bounds.width, center.y };
            case AxisDirection::West:
                return { bounds.x, center.y };
            default:
                return center;
        }
    };
    const Vector2 in = edge(ray.in);
    const Vector2 out = edge(ray.out);

    // Weak rays fade, but never vanish entirely..
    float strength = std::clamp(
        static_cast<float>(ray.strength / ray.strengthMax), 0.25F, 1.F);
    float pulseBase = 0.9F;
    float pulseAmp = 0.1F;
    float pulseFreq = 4.F;
    strength *= pulseBase + (pulseAmp * sinf(GetTime() * pulseFreq));
    const Color baseColor = ToRaylibColor(ray.color);

    // Rounded capsule: in => center => out
    auto stroke = [&](float thick, Color c)
    {
        DrawLineEx(in, center, thick, c);
        DrawLineEx(center, out, thick, c);
        DrawCircleV(center, thick / 2, c);
    };

    // Soft glow (wide + faint => narrow + bright), then a hot core..
    stroke(32.F, Fade(baseColor, 0.10F * strength));
    stroke(24.F, Fade(baseColor, 0.25F * strength));
    stroke(18.F, Fade(baseColor, 0.40F * strength));
    stroke(12.F, Fade(baseColor, 0.60F * strength));
    stroke(8.F, Fade(ColorBrightness(baseColor, 0.5F), 0.75F * strength));
}

void Game::drawLightField() const
{
    BeginBlendMode(BLEND_ADDITIVE);

    lightField_.grid->ForEach(
        [&](const LightCell &cell, Coords coords)
        {
            for (const LightRay &ray : cell.rays)
                DrawLightRay(ray, board_->GetRect(coords));
        });

    EndBlendMode();
}

void Game::draw() const
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    board_->Draw();

    drawLightField();

    // Draw pieces on top of light..
    board_->ForEach([&](const Cell &cell, Coords coords)
        { DrawPiece(cell.piece, board_->GetRect(coords)); });

    // Draw bag_ w/ Slots..
    bag_->ForEach([&](const Slot &slot, Coords coords)
        { DrawInventorySlot(slot, bag_->GetRect(coords)); });

    // Draw drag_;
    if (drag_.held)
    {
        const Vector2 m = GetMousePosition();
        Rectangle bounds = {
            m.x,
            m.y,
            GRID_CELL_SIZE,
            GRID_CELL_SIZE,
        };
        DrawPiece(*drag_.held, bounds);
    }

    DrawFPS(SCREEN_WIDTH - 90, 10);
    EndDrawing();
}
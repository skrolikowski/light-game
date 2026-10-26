#include <algorithm>
#include <map>
#include <raylib.h>

#include "Def.h"
#include "Game.h"
#include "Piece.h"

void Game::LoadLevel(std::string path)
{
    level_ = Level{ path };

    board_ = std::make_unique<Grid<Cell>>(Grid<Cell>{
        Vector2{ 64.F, 64.F },
        level_.GetRows(),
        level_.GetCols(),
        Vector2{ GRID_CELL_SIZE, GRID_CELL_SIZE },
    });

    // Add pieces to board..
    std::map<Coords, Piece> pieces = level_.GetBoardPieces();
    for (auto [coords, piece] : pieces)
        board_->GetItem(coords)->piece = piece;

    // Add inventory..
    Vector2 bagOrigin{ SCREEN_WIDTH - (GRID_SLOT_WIDTH + 64.F), 64.F };
    Vector2 bagSize{ GRID_SLOT_WIDTH, GRID_SLOT_HEIGHT };
    std::map<Coords, Slot> slots = level_.GetSlotPieces();
    int numRows = static_cast<int>(slots.size());
    int numCols = 1;
    bag_ = std::make_unique<Grid<Slot>>(
        Grid<Slot>{ bagOrigin, numRows, numCols, bagSize });

    // Add pieces to inventory..
    bag_->ForEach(
        [&](Slot &slot, Coords coords)
        {
            if (slots.contains(coords))
                slot = slots[coords];
        });

    // Immediately simulate board..
    lightField_ = Simulate(board_.get());
    lightFieldResult_ = CheckLightField(board_.get(), lightField_);
}

LightFieldResult Game::CheckLightField(
    const Grid<Cell> *grid, const LightField &field)
{
    LightFieldResult result;
    result.isStable = field.isStable;
    grid->ForEach(
        [&](const Cell &cell, Coords coords)
        {
            const auto *targetPiece = std::get_if<Target>(&cell.piece);
            if (targetPiece != nullptr)
            {
                result.numTargets++;
                std::vector<LightRay> rays = field.grid->GetItem(coords)->rays;

                for (const auto &ray : rays)
                    if (ray.strength > 0 && ray.color == targetPiece->color)
                        result.numTargetsLit++;
            }
        });
    return result;
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
                TransformPiece(piece, lc.rays);
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
                TransformPiece(piece, lc.rays);

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
            const Coords nextCoords = Grid<Cell>::Translate(ray.out, coords);
            if (Grid<Cell>::InBounds(grid, nextCoords))
            {
                const int nextStrength = ray.strength - 1;
                LightRay nextRay{
                    .color = ray.color,
                    .strength = nextStrength,
                    .strengthMax = nextStrength,
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
    if (TrackMouseInput(GetMousePosition()))
    {
        // Update status..
        lightField_ = Simulate(board_.get());
        lightFieldResult_ = CheckLightField(board_.get(), lightField_);
    }

    if (lightFieldResult_.isSolved())
    {
        // TODO: do something
    }
}

bool Game::TrackMouseInput(const Vector2 &v)
{
    bool changed = false;

    // --
    const Coords coords = bag_->GetCoordsAt(v);
    Cell *cell = board_->GetItemAt(v);
    Slot *slot = bag_->GetItemAt(v);

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
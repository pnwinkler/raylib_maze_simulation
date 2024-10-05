// Generate a maze using the recursive backtracking algorithm and display it graphically

#include "recursive_backtracking.h"
#include <algorithm>
#include <deque>
#include <future>
#include <random>
#include "../../lib/raylib.h"
#include "../constants.cpp"
#include "../utils.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

using namespace constants;
using namespace utils;

//------------------------------------------------------------------------------
// Set up requisite data structures and variables to aid with maze creation
//------------------------------------------------------------------------------

bool _firstSimulationTick = true;

// Holds tasks required for simulation, in a queue, to be called later
std::deque<std::packaged_task<bool()>> taskDeque;

// Holds the X and Y locations of the most recent edit on the grid
// Each edit can affect 0-2 locations inclusive.
struct mostRecentGridEdit {
    int x0;
    int x1;
    int y0;
    int y1;
};
struct mostRecentGridEdit mrge;

// Progress the state of the maze generation by one tick. If the tick does not effect a visual change,
// then execute subsequent ticks, until the state of the maze changes as a result.
void rb::simulationTick(utils::gridType* grid) {
    if (_firstSimulationTick) {
        _firstSimulationTick = false;
        XY start = {0, 0};
        rb::_carvePassagesFrom(start, grid);
        return;
    }

    // The changeEffected variable is used to indicate whether a change to the grid's state has occurred.
    // It lets us fast forward over tasks that don't change grid state (which is a better user experience IMO).
    // Recursive backtracking inherently performs ticks that don't change maze state.
    auto changeEffected = false;

    while (!changeEffected && !taskDeque.empty()) {
        std::packaged_task<bool()> task = std::move(taskDeque.front());
        taskDeque.pop_front();
        auto fut = task.get_future();
        task();
        changeEffected = fut.get();
    }
}

void rb::_wasmFuncToDisplayMazeBuildSteps(void* arg) {
    gridType* grid_ptr = static_cast<gridType*>(arg);

    BeginDrawing();
    rb::_simulationDraw(grid_ptr);
    rb::simulationTick(grid_ptr);
    EndDrawing();
    if (taskDeque.empty()) {
        // repeat one last time, to ensure the final state (e.g. task count) is displayed, then stop
        BeginDrawing();
        rb::simulationTick(grid_ptr);
        rb::_simulationDraw(grid_ptr);
        EndDrawing();
    }
}

void rb::_nonWasmFuncToDisplayMazeBuildSteps(void* arg) {
    // We need to take void* as an argument, so that our WASM and non-WASM funcs can have the same signature
    // And we need void* because that's what emscripten's set main loop function expects
    gridType* grid_ptr = static_cast<gridType*>(arg);

    SetTargetFPS(FPS_GENERATING);
    while (!WindowShouldClose())  // Detect window close button or ESC key
    {
        BeginDrawing();
        rb::_simulationDraw(grid_ptr);
        rb::simulationTick(grid_ptr);
        EndDrawing();
    }
    CloseWindow();
}

// Generates the maze instantly, with no animation
void rb::generateMazeInstantlyNoDisplay(utils::gridType* grid) {
    do {
        simulationTick(grid);
    } while (!taskDeque.empty());
}

// Helps draw grid state in GUI. Expects an existing window.
void rb::_simulationDraw(utils::gridType* grid) {
    ClearBackground(RAYWHITE);
    DrawText(TextFormat("Tasks: %01i", taskDeque.size()), 10, 10, 10, MAROON);
    for (int y = 0; y < grid->size(); y++) {
        for (int x = 0; x < grid->at(0).size(); x++) {
            int val = grid->at(y).at(x);

            // Draw the walls between cells
            if (val != SOUTH && !(y < grid->size() - 1 && grid->at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            if (val != EAST && !(x < grid->at(0).size() - 1 && grid->at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);

            // Draw rectangles to help the user identify the most recent cells to have changed
            if (mrge.x0 == x && mrge.y0 == y)
                DrawRectangle(mrge.x0 * CELLWIDTH, mrge.y0 * CELLHEIGHT, CELLWIDTH, CELLHEIGHT, PINK);
            if (mrge.x1 == x && mrge.y1 == y)
                DrawRectangle(mrge.x1 * CELLWIDTH, mrge.y1 * CELLHEIGHT, CELLWIDTH, CELLHEIGHT, PINK);
        }
    }
}

//------------------------------------------------------------------------------
// The algorithm itself
//------------------------------------------------------------------------------

// Connects two cells in the grid, subject to constraints. Returns true if it changed the grid's state, else
// false.
bool rb::_carvePassagesFrom(const XY& start, gridType* grid) {
    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    for (const auto& direction : directions) {
        XY neighbor = {start.x + DX[direction], start.y + DY[direction]};
        bool targetInBounds = inBounds(*grid, neighbor);

        if (targetInBounds && grid->at(neighbor.y).at(neighbor.x) == 0) {
            // Queueing tasks lets us more easily control the interval between simulation
            // steps, which makes rendering the state easier
            std::packaged_task<bool()> task(std::bind([start, neighbor, direction, grid]() mutable {
                return _carvePassagesHelper(start, neighbor, direction, grid);
            }));
            if (taskDeque.size() < QUEUE_LENGTH_LIMIT) {
                taskDeque.push_front(std::move(task));
            }
        }
    }
    return false;
}

// Attempts to connect source and target cells within the grid. Returns true if it changed the grid's state, else
// false.
bool rb::_carvePassagesHelper(const XY& start, const XY& target, const int direction, gridType* grid) {
    bool targetInBounds = inBounds(*grid, target);
    bool targetHasNoConnections = grid->at(target.y).at(target.x) == 0;

    if (!(targetInBounds && targetHasNoConnections)) {
        return false;
    }

    mrge.y0 = -1;
    mrge.x0 = -1;
    mrge.x1 = -1;
    mrge.y1 = -1;

    // Don't overwrite existing connections
    if (grid->at(start.y).at(start.x) == 0) {
        grid->at(start.y).at(start.x) = direction;
        mrge.x0 = start.x;
        mrge.y0 = start.y;
    }
    grid->at(target.y).at(target.x) = OPPOSITE[direction];
    mrge.x1 = target.x;
    mrge.y1 = target.y;

    std::packaged_task<bool()> task(std::bind([target, grid]() { return _carvePassagesFrom(target, grid); }));
    if (taskDeque.size() < QUEUE_LENGTH_LIMIT) {
        taskDeque.push_front(std::move(task));
    }
    return true;
}

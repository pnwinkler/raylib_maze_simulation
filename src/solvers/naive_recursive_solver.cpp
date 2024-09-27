// Given a grid, find a contiguous line between the defined start point and end point
// This solver, for every cell, moves in a random direction until it finds the target while never visiting a cell twice.
// If it reaches a dead end, it jumps back to the last cell whose neighbors it has not all visited, and then resumes
// execution.

#include "naive_recursive_solver.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
#include "../../lib/raylib.h"  // For WASM
#include "../constants.cpp"
#include "../utils.h"
#include "deque"
#include "unordered_set"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

using namespace constants;
using namespace utils;

// Store the indices of all cells that we have examined.
static std::unordered_set<int> g_indicesChecked = {};

// Store the details of each cell visited, in the order they were visited.
static std::deque<utils::XY> g_locationsInOrderVisited = {};

// The number of tasks queued at the time when the cell at the same index
// in g_locationsInOrderVisited was visited by the algorithm
static std::vector<int> g_taskCount = {};

// Perform the next step of the algorithm. Return true if target was found, else false.
// Effectively, for every location to check, we check if it can connect to a neighboring cell.
// If it can, then we add that cell to the list of locations to check.
bool ns::nextStep(gridType& grid, XY target, std::deque<XY>& locationsToCheck) {
    XY origin = locationsToCheck.front();
    locationsToCheck.pop_front();
    g_locationsInOrderVisited.push_back(origin);
    g_taskCount.push_back(locationsToCheck.size() + 1);
    g_indicesChecked.insert((origin.y * grid.size()) + origin.x);

    assert(inBounds(grid, origin));

    if (origin == target) {
        std::cout << "FOUND target at " << origin.x << ',' << origin.y << '\n';
        g_taskCount.back() = 0;
        return true;
    }

    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    for (const auto& direction : directions) {
        XY neighbor = {origin.x + DX[direction], origin.y + DY[direction]};
        if (!inBounds(grid, neighbor)) {
            continue;
        };
        bool indexChecked = g_indicesChecked.contains((neighbor.y * grid.size()) + neighbor.x);
        if (indexChecked) {
            continue;
        }

        // Either our cell points to that cell or that cell points to our cell, or both
        bool noWallBetween = ((grid[origin.y][origin.x] & direction) != 0) ||
                             ((grid[neighbor.y][neighbor.x] & OPPOSITE[direction]) != 0);

        if (noWallBetween) {
            locationsToCheck.push_back(neighbor);
        }
    }

    return false;
}

// Given a valid maze, find a path within that maze, connecting the start and end locations,
// while respecting maze walls.
void ns::solve(gridType& grid, XY startLoc, XY endLoc) {
    if (!inBounds(grid, startLoc))
        throw std::invalid_argument("Start location out of grid bounds");
    if (!inBounds(grid, endLoc))
        throw std::invalid_argument("End location out of grid bounds");

    g_indicesChecked.reserve(ROWS * COLS);

    // Repeatedly execute the next step of the algorithm, until we find the target cell.
    bool found = false;
    std::deque<XY> locationsToCheck = {startLoc};
    while (!found && (g_indicesChecked.size() < grid.size() * grid.at(0).size()) && locationsToCheck.size() > 0) {
        found = ns::nextStep(grid, endLoc, locationsToCheck);
    }

    if (!found) {
        std::cerr << "Failed to find solution connecting points (" << startLoc.y << "," << startLoc.x << ") and ("
                  << endLoc.x << "," << endLoc.y << ")" << std::endl;
    }
}

// Animate the solution to the maze. If the maze has not yet been solved, then this function
// solves it immediately.
void ns::animateSolution(gridType& grid) {
    if (g_locationsInOrderVisited.size() == 0) {
        // No attempt has yet been made to solve the maze
        ns::solve(grid, solverStart, solverEnd);
    }

    int locationIndex = 0;
    SetTargetFPS(FPS_SOLVING);
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (locationIndex < g_locationsInOrderVisited.size()) {
            ns::_solverDraw(grid, locationIndex);
            locationIndex++;
        }
        EndDrawing();
    }
    CloseWindow();
}

// This helper function draws the grid's state in GUI. It expects an existing window.
void ns::_solverDraw(gridType& grid, int locationIdx) {
    ClearBackground(RAYWHITE);

    // This is the location being evaluated by the algorithm at this particular stage.
    auto checkedLocation = g_locationsInOrderVisited.at(locationIdx);

    // This is the maze exit.
    auto mazeEndpoint = g_locationsInOrderVisited.back();

    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            // Draw the maze exit.
            // The offsets are intended to stop this shape from being drawn over the walls of the maze
            DrawRectangle(mazeEndpoint.x * CELLWIDTH, mazeEndpoint.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                          mazeEndpointColor);

            // Draw the location currently being checked.
            DrawRectangle(checkedLocation.x * CELLWIDTH, checkedLocation.y * CELLHEIGHT + 1, CELLWIDTH - 1,
                          CELLHEIGHT - 1, cellFocusColor);

            // Add indication of previously visited cells
            for (int i = 0; i < locationIdx; i++) {
                Color clr = utils::gradateColor(cellFocusColor, RAYWHITE, i, locationIdx);
                auto visitedLoc = g_locationsInOrderVisited.at(i);
                DrawRectangle(visitedLoc.x * CELLWIDTH, visitedLoc.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                              clr);
            }

            // Draw the walls between cells
            int cell_val = grid.at(y).at(x);
            bool origin_points_east = (cell_val & EAST) != 0;
            bool origin_points_south = (cell_val & SOUTH) != 0;
            bool neighbor_points_west = x + DX[EAST] < grid.at(0).size() && (grid.at(y)[x + DX[EAST]] & WEST) != 0;
            bool neighbor_points_north = y + DY[SOUTH] < grid.size() && (grid.at(y + DY[SOUTH])[x] & NORTH) != 0;
            if (!origin_points_east && !neighbor_points_west) {
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, wallColor);
            }
            if (!origin_points_south && !neighbor_points_north) {
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, wallColor);
            }
        }
    }
    DrawText(("Tasks " + std::to_string(g_taskCount.at(locationIdx))).c_str(), 10, 10, 10, MAROON);
}

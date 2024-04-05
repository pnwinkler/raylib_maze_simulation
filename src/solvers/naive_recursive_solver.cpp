// Given a grid, find a contiguous line between the defined start point and end point
// This solver, for every cell, moves in a random direction until it finds the target while never visiting a cell twice.
// If it reaches a dead end, it jumps back to the last cell whose neighbors it has not all visited, and then resumes
// execution.

#include "naive_recursive_solver.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thread>
#include "../../lib/raylib.h"  // For WASM
#include "../constants.cpp"
#include "../generators/recursive_backtracking.h"
#include "../utils.h"
#include "deque"
#include "unordered_set"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

using namespace constants;
using namespace utils;

static std::unordered_set<int> indicesChecked = {};
static std::deque<utils::XY> locationsInOrderVisited = {};

bool ns::nextStep(gridType& grid, XY target, std::deque<XY>& locationsToCheck) {
    // Perform the next step of the algorithm. Return true if target was found, else false.

    XY currentLocation = locationsToCheck.front();
    locationsToCheck.pop_front();
    locationsInOrderVisited.push_back(currentLocation);
    indicesChecked.insert((currentLocation.y * grid.size()) + currentLocation.x);

    assert(utils::inBounds(grid, currentLocation));

    if (currentLocation.y == target.y && currentLocation.x == target.x) {
        std::cout << "FOUND at " << currentLocation.x << ',' << currentLocation.y << '\n';
        return true;
    }

    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    for (const auto& direction : directions) {
        XY neighbor = {currentLocation.x + DX[direction], currentLocation.y + DY[direction]};
        bool targetInBounds = inBounds(grid, neighbor);
        bool indexChecked = indicesChecked.contains((neighbor.y * grid.size()) + neighbor.x);
        // Either our cell points to that cell or that cell points to our cell
        bool noWallBetween = targetInBounds && ((grid[currentLocation.y][currentLocation.x] & direction != 0) ||
                                                (grid[neighbor.y][neighbor.x] & OPPOSITE[direction] != 0));

        if (!indexChecked && noWallBetween) {
            locationsToCheck.push_back(neighbor);
        }
    }

    return false;
}

void ns::solve(gridType& grid, XY startLoc, XY endLoc) {
    // Given a valid maze, find a path within that maze, connecting the start and end locations,
    // while respecting maze walls.
    if (!inBounds(grid, startLoc))
        throw std::invalid_argument("Start location out of grid bounds");
    if (!inBounds(grid, endLoc))
        throw std::invalid_argument("End location out of grid bounds");

    indicesChecked.reserve(ROWS * COLS);
    std::deque<XY> locationsToCheck = {};

    bool found = false;
    locationsToCheck.push_front(startLoc);
    indicesChecked.insert(startLoc.y * grid.size() + startLoc.x);

    while (!found && (indicesChecked.size() < grid.size() * grid.at(0).size()) && locationsToCheck.size() > 0) {
        found = ns::nextStep(grid, endLoc, locationsToCheck);
    }

    if (!found) {
        std::cerr << "Failed to find solution connecting points (" << startLoc.y << "," << startLoc.x << ") and ("
                  << endLoc.x << "," << endLoc.y << ")" << std::endl;
    }
}

void ns::animateSolution(gridType& grid) {
    if (locationsInOrderVisited.size() == 0) {
        // No attempt has yet been made to solve the maze
        ns::solve(grid, solverStart, solverEnd);
    }

    int locationIndex = 0;
    SetTargetFPS(FPS_SOLVING);
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (locationIndex < locationsInOrderVisited.size()) {
            ns::_solverDraw(grid, locationIndex);
            locationIndex++;
        }
        EndDrawing();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS_SOLVING));
    }
    CloseWindow();
}

void ns::_solverDraw(gridType& grid, int locationIdx) {
    // Helper function, to draw grid state in GUI. Expects an existing window.

    ClearBackground(RAYWHITE);

    auto checkedLocation = locationsInOrderVisited.at(locationIdx);

    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            // The offsets are intended to stop this shape from being drawn over the walls of the maze
            auto mazeEndpoint = locationsInOrderVisited.back();
            DrawRectangle(mazeEndpoint.x * CELLWIDTH, mazeEndpoint.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                          LIGHTGRAY);
            DrawRectangle(checkedLocation.x * CELLWIDTH, checkedLocation.y * CELLHEIGHT + 1, CELLWIDTH - 1,
                          CELLHEIGHT - 1, PURPLE);

            // Add indication of previously visited cells
            for (int i = 0; i < locationIdx; i++) {
                Color clr = utils::gradateColor(PURPLE, RAYWHITE, i, locationIdx);
                auto visitedLoc = locationsInOrderVisited.at(i);
                DrawRectangle(visitedLoc.x * CELLWIDTH, visitedLoc.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                              clr);
            }
            DrawText("Solver", 5, 5, 6, RED);

            // Draw the walls
            int val = grid.at(y).at(x);
            if ((val & SOUTH == 0) && !(y < grid.size() - 1 && grid.at(y + DY[SOUTH])[x] & NORTH != 0))
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            if ((val & EAST == 0) && !(x < grid.at(0).size() - 1 && grid.at(y)[x + DX[EAST]] & WEST != 0))
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
        }
    }
}
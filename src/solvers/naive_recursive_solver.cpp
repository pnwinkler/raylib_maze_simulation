// Given a grid, find a contiguous line between the defined start point and end point
// This solver, for every cell, moves in a random direction until it finds the target while never visiting a cell twice.
// If it reaches a dead end, it jumps back to the last cell whose neighbors it has not all visited, and then resumes
// execution.

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thread>
#include "../../lib/raylib.h"  // For WASM
#include "naive_recursive_solver.h"
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

std::unordered_set<int> indicesChecked = {};
std::deque<utils::XY> locationsInOrderVisited = {};

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
        bool noWallBetween = targetInBounds && (grid[currentLocation.y][currentLocation.x] == direction ||
                                                grid[neighbor.y][neighbor.x] == OPPOSITE[direction]);

        if (!indexChecked && noWallBetween) {
            locationsToCheck.push_back(neighbor);
        }
    }

    return false;
}

void ns::naiveSolver(gridType& grid, XY startLoc, XY endLoc) {
    // Given a valid maze, find a path within that maze, connecting the start and end locations,
    // while respecting maze walls.
    indicesChecked.reserve(ROWS* COLS);
    if (endLoc.x >= grid.at(0).size() || endLoc.y >= grid.size()) {
        throw std::invalid_argument("Target location out of grid bounds");
    }
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

void ns::SolverUpdateDrawFrame(void) {
    // TODO: get this function for WASM working, then get the transition from maze generation to mazedisplay solving working, too
    //     That probably means interfacing nicely with the new simGui code
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

    EndDrawing();
    //----------------------------------------------------------------------------------
}

void ns::animateSolution(gridType& grid) {
    auto dims = utils::calculateCanvasDimensions();
    InitWindow(dims.x, dims.y, "Maze solving: naive recursion");

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(SolverUpdateDrawFrame, 0, 1);
#else
    int locationIndex = 0;
    SetTargetFPS(FPS_SOLVING);
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (locationIndex < locationsInOrderVisited.size()) {
            ns::_solverDraw(grid, locationIndex);
            locationIndex++;
        }
        EndDrawing();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS_SOLVING));
    }
#endif
}

Color ns::gradateColor(Color start, Color target, int i, int locationIdx) {
    // Apply a function to return a color between the start and target colors
    auto d1 = target.r - start.r;
    auto d2 = target.g - start.g;
    auto d3 = target.b - start.b;
    auto d4 = target.a - start.a;

    // For i values close to locationIdx, return more similar colors than for i values further from it.
    // Reasoning: I believe it's visually more appealing (and less distracting) to use fairly uniform colors for cells
    // visited long ago. This uniformity also effectively exaggerates the constrast between recently visited cells,
    // making it easier at a glance to reconstruct the recent path taken.
    float multiplier;
    if (locationIdx - i < 3) {
        // Gradate normally
        multiplier = (float)i / locationIdx;
    } else {
        multiplier = (float)i / (locationIdx * 1.5);
    }

    int d1Col = target.r - (d1 * (multiplier));
    int d2Col = target.g - (d2 * (multiplier));
    int d3Col = target.b - (d3 * (multiplier));
    int d4Col = target.a - (d4 * (multiplier));
    Color clr = Color(d1Col, d2Col, d3Col, d4Col);
    return clr;
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
                Color clr = gradateColor(PURPLE, RAYWHITE, i, locationIdx);
                auto visitedLoc = locationsInOrderVisited.at(i);
                DrawRectangle(visitedLoc.x * CELLWIDTH, visitedLoc.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                              clr);
            }
            DrawText("Naive solver", 5, 5, 6, RED);

            // Draw the walls
            int val = grid.at(y).at(x);
            if (val != SOUTH && !(y < grid.size() - 1 && grid.at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            if (val != EAST && !(x < grid.at(0).size() - 1 && grid.at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
        }
    }
}

// int main() {
//     // TODO:
//     //  improve the graphical display by:
//     //    consider adding stats, like % cells visited, steps performed, dead ends encountered, etc
//     //  make a proximity based recursive solver once done with this solver
//     srand(time(NULL));
//     indicesChecked.reserve(ROWS * COLS);
//     gridType grid = utils::generateGrid(ROWS, COLS);

//     rb::recursive_backtracking rb;
//     // Enable one of the following lines only. One shows the maze-to-be-solved being built, whereas the other builds
//     it
//     // but doesn't show it
//     rb.generateMazeInstantlyNoDisplay(&grid);
//     // rb.displayMazeBuildSteps(&grid);

//     // these should be 0 indexed
//     utils::XY start = {0, 0};
//     utils::XY end = {ROWS - 1, COLS - 1};
//     naiveSolver(grid, start, end);

//     animateSolution(grid);
// }

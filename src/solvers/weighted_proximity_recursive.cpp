// Given a grid, find a contiguous line between the defined start point and end point.
// This algorothm uses a weighted proximity approach, where the next cell to visit is determined based
// on its distance from the target (calculated as the sum of the absolute differences between the cell
// and the target cell).
// This algorithm is recursive, and will continue to visit cells until the target is found,
// or all cells have been visited.

#include "weighted_proximity_recursive.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>
#include "../../lib/raylib.h"
#include "../constants.cpp"
#include "../utils.h"
#include "deque"
#include "map"
#include "unordered_set"
#include "vector"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#include <memory>

using namespace constants;

static std::unordered_set<int> g_indicesChecked = {};
static std::deque<XY> g_locationsInOrderVisited = {};
static const Color SCORE_COLOR = {170, 61, 155, 155};

// Calculate the score for a given cell
int ws::calculateScore(const XY& cell, const XY& mazeFinish) {
    int diffX = mazeFinish.x - cell.x;
    int diffY = mazeFinish.y - cell.y;
    if (diffX < 0) {
        diffX *= -1;
    }
    if (diffY < 0) {
        diffY *= -1;
    }
    return diffX + diffY;
}

// From the remaining scores, pop any one of the best scoring elements
XY ws::popBestScorer(ws::tScores& remainingScores) {
    auto lowest = remainingScores.begin();
    auto key = lowest->first;
    auto& vals = lowest->second;
    XY origin = *vals.begin();
    vals.erase(vals.begin());
    if (vals.empty()) {
        remainingScores.erase(key);
    }
    return origin;
}

// Insert the cell into the remainingScores vector, in the correct position, based on the score. If the cell is
// already in the remainingScores vector, then do nothing.
void ws::insertIntoScores(const XY& cell, const int score, ws::tScores& remainingScores) {
    if (remainingScores.find(score) != remainingScores.end()) {
        // append the cell into the remaining scores data
        std::cout << "DEBUG: score group for cell (" << cell.x << ',' << cell.y << ") already exists\n";
        // std::cout << "Adding (" << cell.x << ',' << cell.y << ") to remainingScores[" << x << "]\n";
        remainingScores[score].insert(cell);
    } else {
        // add a new group
        std::cout << "DEBUG: adding cell (" << cell.x << ',' << cell.y << ") to new score group\n";
        remainingScores[score] = {cell};
    }
}

void printRemainingScores(const ws::tScores& remainingScores) {
    for (const auto& pair : remainingScores) {
        std::cout << "Score=" << pair.first << " for cells: ";
        for (const auto& cell : pair.second) {
            std::cout << "(x=" << cell.x << ",y=" << cell.y << ')' << ' ';
        }
        std::cout << '\n';
    }
}

// Perform the next step of the algorithm. Return True if the maze is solved, else False.
bool ws::nextStep(gridType& grid, const XY& target, tScores& remainingScores) {
    // TODO: make this function idempotent (atm, we return True only once)
    // Basically, we pop any cell with the best score, and check if its location equals that
    // of our target cell. If it does, then return True (we've solved the maze).
    // Else false. Then add all unchecked neighbors to the list of cells to check.

    if (remainingScores.size() == 0) {
        throw std::runtime_error("Error: remainingScores expected to have at least one element");
    }

    // Select our origin cell, based on score
    XY origin = popBestScorer(remainingScores);
    printRemainingScores(remainingScores);
    std::cout << "DEBUG: origin=(" << origin.x << ',' << origin.y << ")\n";

    if (g_indicesChecked.contains((origin.y * grid.size()) + origin.x)) {
        return false;
    }

    assert(inBounds(grid, origin));
    g_locationsInOrderVisited.push_back(origin);
    g_indicesChecked.insert((origin.y * grid.size()) + origin.x);

    if (origin.y == target.y && origin.x == target.x) {
        std::cout << "FOUND at " << origin.x << ',' << origin.y << '\n';
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
        // Check if both cells points to each other
        bool originPointsInDirection = grid[origin.y][origin.x] & direction != 0;
        bool neighborPointsInOppositeDirection = grid[neighbor.y][neighbor.x] & OPPOSITE[direction] != 0;
        bool noWallBetween = originPointsInDirection && neighborPointsInOppositeDirection;

        if (!indexChecked && noWallBetween) {
            // Neighbor is a potentially valid match
            int score = calculateScore(neighbor, target);
            insertIntoScores(neighbor, score, remainingScores);
        }
    }

    return false;
}

// Given a valid maze, find a path within that maze, connecting the start and end locations,
// while respecting maze walls.
void ws::solve(gridType& grid, const XY& startLoc, const XY& endLoc) {
    if (!inBounds(grid, startLoc))
        throw std::invalid_argument("Start location out of grid bounds");
    if (!inBounds(grid, endLoc))
        throw std::invalid_argument("End location out of grid bounds");

    tScores remainingScores = {};

    g_indicesChecked.reserve(ROWS * COLS);
    int score = calculateScore(startLoc, endLoc);
    insertIntoScores(startLoc, score, remainingScores);

    bool found = false;
    while (!found) {
        found = nextStep(grid, endLoc, remainingScores);
    }
}

void ws::animateSolution(gridType& grid) {
    if (g_locationsInOrderVisited.size() == 0) {
        // No attempt has yet been made to solve the maze
        ws::solve(grid, solverStart, solverEnd);
    }

    int locationIndex = 0;
    SetTargetFPS(FPS_SOLVING);
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (locationIndex < g_locationsInOrderVisited.size()) {
            _solverDraw(grid, locationIndex);
            locationIndex++;
        }
        EndDrawing();
    }
    CloseWindow();
}

// Helper function, to draw grid state in GUI. Expects an existing window.
void ws::_solverDraw(const gridType& grid, const int locationIdx) {
    ClearBackground(RAYWHITE);
    auto checkedLocation = g_locationsInOrderVisited.at(locationIdx);

    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            // The offsets are intended to stop this shape from being drawn over the walls of the maze
            auto mazeEndpoint = g_locationsInOrderVisited.back();
            DrawRectangle(mazeEndpoint.x * CELLWIDTH, mazeEndpoint.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                          LIGHTGRAY);
            DrawRectangle(checkedLocation.x * CELLWIDTH, checkedLocation.y * CELLHEIGHT + 1, CELLWIDTH - 1,
                          CELLHEIGHT - 1, PURPLE);

            // Add indication of previously visited cells
            for (int i = 0; i < locationIdx; i++) {
                Color clr = utils::gradateColor(PURPLE, RAYWHITE, i, locationIdx);
                auto visitedLoc = g_locationsInOrderVisited.at(i);
                DrawRectangle(visitedLoc.x * CELLWIDTH, visitedLoc.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                              clr);
            }

            // Draw the walls
            int val = grid.at(y).at(x);
            if (val != SOUTH && !(y < grid.size() - 1 && grid.at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            if (val != EAST && !(x < grid.at(0).size() - 1 && grid.at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);

            if (constants::displayScores) {
                // Add the score to the cell
                int score = calculateScore({x, y}, solverEnd);
                DrawText(std::to_string(score).c_str(), x * CELLWIDTH + 5, y * CELLHEIGHT + 5, 6, SCORE_COLOR);
            }

            // Put this last, so it gets drawn over other objects
            DrawText("Solver", 5, 5, 6, RED);
        }
    }
}
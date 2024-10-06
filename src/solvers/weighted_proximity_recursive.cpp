// Given a grid, find a contiguous line between the defined start point and end point.
// This algorothm uses a weighted proximity approach, where the next cell to visit is determined based
// on its distance from the target (calculated as the sum of the absolute differences between the cell
// and the target cell).
// This algorithm is recursive, and will continue to visit cells until the target is found,
// or all cells have been visited.

#include "weighted_proximity_recursive.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include "../../lib/raylib.h"
#include "../constants.cpp"
#include "../utils.h"
#include "deque"
#include "map"
#include "unordered_set"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

using namespace constants;

static std::unordered_set<int> g_indicesChecked = {};
static std::deque<XY> g_locationsInOrderVisited = {};
static const Color SCORE_COLOR = {170, 61, 155, 155};
static std::vector<int> g_taskCount = {};

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
    const auto lowest = remainingScores.begin();
    const auto key = lowest->first;
    auto& vals = lowest->second;
    const XY origin = *vals.begin();
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
        std::cout << "DEBUG: adding cell (" << cell.x << ',' << cell.y << ") to existing score group\n";
        remainingScores[score].insert(cell);
    } else {
        // add a new group
        std::cout << "DEBUG: adding cell (" << cell.x << ',' << cell.y << ") to new score group\n";
        remainingScores[score] = {cell};
    }
}

void printRemainingScores(const ws::tScores& remainingScores) {
    for (const auto& [score, cells] : remainingScores) {
        std::cout << "Score=" << score << " for cells: ";
        for (const auto& cell : cells) {
            std::cout << "(x=" << cell.x << ",y=" << cell.y << ')' << ' ';
        }
        std::cout << '\n';
    }
}

// Perform the next step of the algorithm. Return True if the maze is solved, else False.
bool ws::nextStep(const gridType& grid, const XY& target, tScores& remainingScores) {
    // Basically, we pop any cell with the best score, and check if its location equals that
    // of our target cell. If it does, then return True (we've solved the maze).
    // Else false. Then add all unchecked neighbors to the list of cells to check.

    if (remainingScores.size() == 0) {
        throw std::runtime_error("Error: remainingScores expected to have at least one element");
    }

    int countLocationsToCheck = 0;
    for (auto const& [key, val] : remainingScores) {
        countLocationsToCheck += val.size();
    }
    g_taskCount.push_back(countLocationsToCheck);

    // Select our origin cell, based on score
    const XY origin = popBestScorer(remainingScores);
    printRemainingScores(remainingScores);
    std::cout << "DEBUG: origin=(" << origin.x << ',' << origin.y << ")\n" << std::endl;

    if (g_indicesChecked.contains((origin.y * grid.size()) + origin.x)) {
        return false;
    }

    assert(inBounds(grid, origin));
    g_locationsInOrderVisited.push_back(origin);
    g_indicesChecked.insert((origin.y * grid.size()) + origin.x);

    if (origin == target) {
        std::cout << "FOUND target at " << origin.x << ',' << origin.y << '\n';
        g_taskCount.back() = 0;
        return true;
    }

    const auto neighbors = utils::returnAccessibleNeighbors(grid, origin, target, g_indicesChecked);
    for (const auto neighbor : neighbors) {
        const int score = calculateScore(neighbor, target);
        insertIntoScores(neighbor, score, remainingScores);
    }

    return false;
}

// Given a valid maze, find a path within that maze, connecting the start and end locations,
// while respecting maze walls.
void ws::solve(const gridType& grid, const XY& startLoc, const XY& endLoc) {
    if (!inBounds(grid, startLoc))
        throw std::invalid_argument("Start location out of grid bounds");
    if (!inBounds(grid, endLoc))
        throw std::invalid_argument("End location out of grid bounds");

    tScores remainingScores = {};

    g_indicesChecked.reserve(ROWS * COLS);
    const int score = calculateScore(startLoc, endLoc);
    insertIntoScores(startLoc, score, remainingScores);

    bool found = false;
    while (!found) {
        found = nextStep(grid, endLoc, remainingScores);
    }
}

void ws::animateSolution(const gridType& grid) {
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
    const auto checkedLocation = g_locationsInOrderVisited.at(locationIdx);

    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            // The offsets are intended to stop this shape from being drawn over the walls of the maze
            const auto mazeEndpoint = g_locationsInOrderVisited.back();
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
            const int val = grid.at(y).at(x);
            if (val != SOUTH && !(y < grid.size() - 1 && grid.at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            if (val != EAST && !(x < grid.at(0).size() - 1 && grid.at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);

            if (constants::displayScores) {
                // Add the score to the cell
                const int score = calculateScore({x, y}, solverEnd);
                DrawText(TextFormat("%01i", score), x * CELLWIDTH + 5, y * CELLHEIGHT + 5, 6, SCORE_COLOR);
            }
        }
    }
    DrawText(TextFormat("Queue len: %01i", g_taskCount.at(locationIdx)), 5, 5, 0, MAROON);
}

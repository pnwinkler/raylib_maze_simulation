// Given a grid, find a contiguous line between the defined start point and end point.
// This algorothm uses a weighted proximity approach, where the next cell to visit is the one with the lowest weight
// (calculated as the sum of the absolute differences between the cell and the target cell). The algorithm is recursive,
// and will continue to visit cells until the target is found, or all cells have been visited.

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
#include "unordered_set"
#include "vector"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif
#include <memory>

using namespace constants;

struct scoreGroup {
    int score;
    std::vector<XY> cells = {};
};
typedef std::vector<scoreGroup> sortedScores;
// Cells and scores for all cells that have not yet been visited. Sorted in ascending order of score.
static std::shared_ptr<sortedScores> remainingScores = std::make_shared<sortedScores>();

static std::unordered_set<int> indicesChecked = {};
static std::deque<XY> locationsInOrderVisited = {};
static const Color SCORE_COLOR = {170, 61, 155, 155};

int ws::calculateWeight(XY cell, XY mazeFinish) {
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

void ws::insertIntoScores(XY cell, int score) {
    // Insert the cell into the remainingScores vector, in the correct position, based on the score. If the cell is
    // already in the remainingScores vector, then do nothing.

    for (int x = remainingScores->size() - 1; x > -1; x--) {
        if (score < remainingScores->at(x).score) {
            continue;
        }

        else if (score == remainingScores->at(x).score) {
            // Score group already exists. Add the cell to it, if it's not already there.
            std::cout << "DEBUG: score group for cell (" << cell.x << ',' << cell.y << ") already exists\n";
            for (const auto& existingCell : remainingScores->at(x).cells) {
                if (existingCell.y == cell.y && existingCell.x == cell.x) {
                    std::cout << "DEBUG: cell (" << cell.x << ',' << cell.y << ") already exists in score group as ("
                              << existingCell.x << ',' << existingCell.y << ")\n";
                    return;
                }
                std::cout << "Adding (" << cell.x << ',' << cell.y << ") to remainingScores[" << x << "]\n";
                remainingScores->at(x).cells.push_back(cell);
            }
            return;
        }

        else if (score > remainingScores->at(x).score) {
            // New score exceeds all remaining scores. Insert it after this element.
            std::cout << "DEBUG: adding cell (" << cell.x << ',' << cell.y << ") to new score group\n";
            scoreGroup sg = {score, {cell}};
            remainingScores->insert(remainingScores->begin() + x + 1, sg);
            return;
        }
    }

    // remainingScores either has 0 elements, or score is less than the lowest score element.
    std::cout << "DEBUG: Adding (" << cell.x << ',' << cell.y << ") to new score group\n";
    std::vector<XY> cells = {cell};
    scoreGroup sg = {score, cells};
    remainingScores->insert(remainingScores->begin(), sg);
    return;
}

void printRemainingScores() {
    for (int i = 0; i < remainingScores->size(); i++) {
        std::cout << "Score=" << remainingScores->at(i).score << " for cells: ";
        for (int j = 0; j < remainingScores->at(i).cells.size(); j++) {
            std::cout << '(' << remainingScores->at(i).cells.at(j).x << ',' << remainingScores->at(i).cells.at(j).y
                      << ')';
        }
        std::cout << '\n';
    }
}

bool ws::nextStep(gridType& grid, XY target) {
    // Perform the next step of the algorithm. Return true if target was found, else false
    // pop any one of the elements with the greatest weight

    if (remainingScores->empty()) {
        throw std::runtime_error("Error: remainingScores expected to have at least one element");
    }

    // Pop any of the lowest score elements
    XY origin = remainingScores->at(0).cells.at(0);
    printRemainingScores();
    std::cout << "DEBUG: origin=(" << origin.x << ',' << origin.y << ")\n";
    std::cout << '\n';
    remainingScores->at(0).cells.erase(remainingScores->at(0).cells.begin());

    if (remainingScores->at(0).cells.empty()) {
        remainingScores->erase(remainingScores->begin());
    }

    if (indicesChecked.contains((origin.y * grid.size()) + origin.x)) {
        return false;
    }

    assert(inBounds(grid, origin));
    locationsInOrderVisited.push_back(origin);
    indicesChecked.insert((origin.y * grid.size()) + origin.x);

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
        bool targetInBounds = inBounds(grid, neighbor);
        bool indexChecked = indicesChecked.contains((neighbor.y * grid.size()) + neighbor.x);
        // Either our cell points to that cell or that cell points to our cell
        bool noWallBetween = targetInBounds && (grid[origin.y][origin.x] == direction ||
                                                grid[neighbor.y][neighbor.x] == OPPOSITE[direction]);

        if (!indexChecked && noWallBetween) {
            int weight = calculateWeight(neighbor, target);
            insertIntoScores(neighbor, weight);
        }
    }

    return false;
}

void ws::solve(gridType& grid, XY startLoc, XY endLoc) {
    // Given a valid maze, find a path within that maze, connecting the start and end locations,
    // while respecting maze walls.
    if (!inBounds(grid, startLoc))
        throw std::invalid_argument("Start location out of grid bounds");
    if (!inBounds(grid, endLoc))
        throw std::invalid_argument("End location out of grid bounds");

    bool found = false;
    int weight = calculateWeight(startLoc, endLoc);
    insertIntoScores(startLoc, weight);

    int nrCells = grid.size() * grid.at(0).size();
    while (!found || (indicesChecked.size() >= nrCells) && remainingScores->size() > 0) {
        found = nextStep(grid, endLoc);
    }

    if (!found) {
        std::cout << "DEBUG: found=" << found << " indicesCheckedSize=" << indicesChecked.size()
                  << " remainingScores->size()= " << remainingScores->size() << '\n';
        std::cerr << "Failed to find solution connecting points (" << startLoc.y << "," << startLoc.x << ") and ("
                  << endLoc.x << "," << endLoc.y << ")" << std::endl;
    }
}

void ws::animateSolution(gridType& grid) {
    if (locationsInOrderVisited.size() == 0) {
        // No attempt has yet been made to solve the maze
        ws::solve(grid, solverStart, solverEnd);
    }

    int locationIndex = 0;
    SetTargetFPS(FPS_SOLVING);
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (locationIndex < locationsInOrderVisited.size()) {
            _solverDraw(grid, locationIndex);
            locationIndex++;
        }
        EndDrawing();
    }
    CloseWindow();
}

void ws::_solverDraw(gridType& grid, int locationIdx) {
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

            // Draw the walls
            int val = grid.at(y).at(x);
            if (val != SOUTH && !(y < grid.size() - 1 && grid.at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            if (val != EAST && !(x < grid.at(0).size() - 1 && grid.at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);

            if (constants::displayScores) {
                // Add the score to the cell
                int score = calculateWeight({x, y}, solverEnd);
                DrawText(std::to_string(score).c_str(), x * CELLWIDTH + 5, y * CELLHEIGHT + 5, 6, SCORE_COLOR);
            }

            // Put this last, so it gets drawn over other objects
            DrawText("Solver", 5, 5, 6, RED);
        }
    }
}
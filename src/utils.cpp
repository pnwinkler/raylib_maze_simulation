#include "utils.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include "cassert"
#include "constants.cpp"
#include "unordered_set"
#include "vector"

using namespace constants;

namespace utils {

// todo: consider replacing with fixed size array
typedef std::vector<std::vector<int>> gridType;

// Create and return a grid-like object, whose every value is set to 0
gridType createEmptyGrid(const int rows, const int cols) {
    gridType grid;
    for (int y = 0; y < rows; y++) {
        std::vector<int> newRow;
        grid.push_back(newRow);
        for (int x = 0; x < cols; x++) {
            grid[y].push_back(0);
        }
    }
    return grid;
}

// Return the minimum X and Y pixel dimensions required by the grid
canvasDims calculateCanvasDimensions() {
    int x = COLS * CELLWIDTH;
    int y = ROWS * CELLHEIGHT;
    return canvasDims{x, y};
}

// Prints the maze to the console.
void displayMazeInConsole(gridType& grid) {
    int height = grid.size();
    int width = grid.at(0).size();
    // Create the top wall
    for (int i = 0; i < width * 2; i++) {
        std::cout << '_';
    }
    std::cout << '\n';

    for (int y = 0; y < height; y++) {
        std::cout << '|';
        for (int x = 0; x < width; x++) {
            int val = grid.at(y).at(x);
            (val == SOUTH || (y < height - 1 && grid.at(y + DY[SOUTH])[x] == NORTH)) ? std::cout << ' '
                                                                                     : std::cout << '_';
            (val == EAST || (x < width - 1 && grid.at(y)[x + DX[EAST]] == WEST)) ? std::cout << ' ' : std::cout << '|';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

// Check if an x,y (both 0 indexed) combination fall within the provided grid's bounds
bool inBounds(const gridType& grid, const int x, const int y) {
    return y < grid.size() && x < grid[0].size();
}

// Check if an XY struct represents a location falling within the provided grid's bounds
bool inBounds(const gridType& grid, const XY& location) {
    return location.y < grid.size() && location.x < grid[0].size();
}

// Apply a function to return a color between the start and target colors
// TODO: determine how this does / should handle negatives
// TODO: rework documentation and signature to make usage/purpose more obvious
Color gradateColor(Color start, Color target, int idx, int maxIdx) {
    auto d1 = target.r - start.r;
    auto d2 = target.g - start.g;
    auto d3 = target.b - start.b;
    auto d4 = target.a - start.a;

    assert(maxIdx > 0 && "maxIdx must be greater than 0");

    // For i values close to locationIdx, return more similar colors than for i values further from it.
    // Reasoning: I believe it's visually more appealing (and less distracting) to use fairly uniform colors for
    // cells visited long ago. This uniformity also effectively exaggerates the constrast between recently visited
    // cells, making it easier at a glance to reconstruct the recent path taken.
    float multiplier;
    if (maxIdx - idx < 3) {
        // Gradate normally
        multiplier = (float)idx / maxIdx;
    } else {
        multiplier = (float)idx / (maxIdx * 1.5);
    }

    int d1Col = target.r - (d1 * (multiplier));
    int d2Col = target.g - (d2 * (multiplier));
    int d3Col = target.b - (d3 * (multiplier));
    int d4Col = target.a - (d4 * (multiplier));
    Color clr = Color(d1Col, d2Col, d3Col, d4Col);
    return clr;
}

std::vector<XY> returnAccessibleNeighbors(const gridType& grid,
                                          const XY& origin,
                                          std::unordered_set<int> g_indicesChecked) {
    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    std::vector<XY> accessibleNeighbors = {};

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
        bool noWallBetween = (((grid[origin.y][origin.x] & direction) != 0) ||
                              ((grid[neighbor.y][neighbor.x] & OPPOSITE[direction]) != 0));

        if (noWallBetween) {
            accessibleNeighbors.push_back(neighbor);
        }
    }
    return accessibleNeighbors;
}

};  // namespace utils

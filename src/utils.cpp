#include "utils.h"
#include <iostream>
#include <vector>
#include "constants.cpp"

using namespace constants;

namespace utils {

typedef std::vector<std::vector<int>> gridType;

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

canvasDims calculateCanvasDimensions() {
    int x = COLS * CELLWIDTH;
    int y = ROWS * CELLHEIGHT;
    return canvasDims{x, y};
}

void displayMazeInConsole(gridType& grid) {
    // Prints the maze to the console.

    // Create top wall
    int height = grid.size();
    int width = grid.at(0).size();
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

bool inBounds(gridType& grid, const int x, const int y) {
    return y < grid.size() && x < grid[0].size();
}

bool inBounds(gridType& grid, const XY& location) {
    return location.y < grid.size() && location.x < grid[0].size();
}


Color gradateColor(Color start, Color target, int idx, int maxIdx) {
    // Apply a function to return a color between the start and target colors
    auto d1 = target.r - start.r;
    auto d2 = target.g - start.g;
    auto d3 = target.b - start.b;
    auto d4 = target.a - start.a;

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


};  // namespace utils
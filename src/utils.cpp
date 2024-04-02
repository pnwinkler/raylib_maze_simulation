#include "utils.h"
#include <iostream>
#include <vector>
#include "constants.cpp"

using namespace constants;

namespace utils {

typedef std::vector<std::vector<int>> gridType;

gridType generateGrid(const int rows, const int cols) {
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

};  // namespace utils
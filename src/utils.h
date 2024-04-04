#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include "../lib/raylib.h"

namespace utils {
// Type definitions
typedef std::vector<std::vector<int>> gridType;
struct XY {
    int x;
    int y;
};
struct canvasDims {
    int x;
    int y;
};

// Function declarations
Color gradateColor(Color start, Color target, int idx, int maxIdx);
bool inBounds(gridType& grid, const XY& location);
bool inBounds(gridType& grid, const int x, const int y);
canvasDims calculateCanvasDimensions();
gridType generateGrid(const int rows, const int cols);
void displayMazeInConsole(gridType& grid);

}  // namespace utils

#endif /* UTILS_H */
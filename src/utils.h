#ifndef UTILS_H
#define UTILS_H

#include <vector>

namespace utils {
// Type definitions
typedef std::vector<std::vector<int>> gridType;
struct XY {int x; int y;};
struct canvasDims {
    int x;
    int y;
};

// Function declarations
inline gridType generateGrid(const int rows, const int cols);
inline canvasDims calculateCanvasDimensions();
inline void displayMazeInConsole(gridType& grid);
inline bool inBounds(gridType& grid, const int x, const int y);
inline bool inBounds(gridType& grid, const XY& location);
}  // namespace utils

#endif /* UTILS_H */
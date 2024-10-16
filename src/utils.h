#ifndef UTILS_H
#define UTILS_H

#include <unordered_set>
#include <vector>
#include "../lib/raylib.h"

namespace utils {
// Type definitions required by multiple files go in this file.

typedef std::vector<std::vector<int>> gridType;
struct XY {
    bool operator==(const XY& rhs) const { return (x == rhs.x) && (y == rhs.y); }
    // bool operator!=(const XY& rhs) const { return !operator==(rhs); }
    // bool operator<(const XY& other) const {
    //     if (x == other.x) {
    //         return y < other.y;
    //     }
    //     return x < other.x;
    // }

    // This is the 0 indexed position on the x axis
    int x;
    // This is the 0 indexed position on the y axis
    int y;
};

struct canvasDims {
    int x;
    int y;
};

// Function declarations
Color gradateColor(Color start, Color target, int idx, int maxIdx);
bool inBounds(const gridType& grid, const XY& location);
bool inBounds(const gridType& grid, const int x, const int y);
canvasDims calculateCanvasDimensions();
gridType createEmptyGrid(const int rows, const int cols);
void displayMazeInConsole(gridType& grid);
std::vector<XY> returnConnectedNeighbors(const gridType& grid,
                                         const XY& origin,
                                         std::unordered_set<int> g_indicesChecked);

}  // namespace utils

// Specialize std::hash for utils::XY
namespace std {
template <>
struct hash<utils::XY> {
    std::size_t operator()(const utils::XY& xy) const {
        // Combine the hash of x and y
        return std::hash<int>()(xy.x) ^ (std::hash<int>()(xy.y) << 1);
    }
};
}  // namespace std

#endif /* UTILS_H */

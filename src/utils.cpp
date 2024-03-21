#include <vector>

// Alias
typedef std::vector<std::vector<int>> gridType;

gridType generateGrid(int rows, int cols) {
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

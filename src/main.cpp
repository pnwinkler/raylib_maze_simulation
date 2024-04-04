// This is our program entry point. From here, we generate the maze, solve it, and then display the results.
// We have our GUI thing, then we should be able to swap in any algorithm generator or solver

/*
How to use:
    - Edit the constants in src/constants.cpp to your liking
    - Execute
*/
#include <ctime>
#include <memory>
#include <stdexcept>
#include "../lib/raylib.h"
#include "constants.cpp"
#include "generators/recursive_backtracking.h"
#include "solvers/naive_recursive_solver.h"
#include "solvers/weighted_proximity_recursive.h"
#include "utils.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

using namespace constants;

int main() {
    srand(time(NULL));
    gridType grid = generateGrid(ROWS, COLS);
    auto dims = utils::calculateCanvasDimensions();

    switch (currentGenerator) {
        case RECURSIVE_BACKTRACKING: {
            // TODO: get WASM display working. The desktop version is now fine.
            InitWindow(dims.x, dims.y, "Maze Generator");
            rb::_nonWasmFuncToDisplayMazeBuildSteps(&grid);
            break;
        }

        case SILENTLY_GENERATE:
            rb::generateMazeInstantlyNoDisplay(&grid);
            break;

        default:
            throw std::invalid_argument("The chosen generator algorithm is not yet implemented");
    };

    switch (currentSolver) {
        case NAIVE_RECURSIVE:
            InitWindow(dims.x, dims.y, "Naive Recursive Solver");
            ns::animateSolution(grid);
            break;

        case WEIGHTED_RECURSIVE:
            InitWindow(dims.x, dims.y, "Proximity Weighted Recursive Solver");
            ws::animateSolution(grid);
            break;

        case SKIP_SOLVING:
            break;

        default:
            throw std::invalid_argument("The chosen solver algorithm is not yet implemented");
    }
}
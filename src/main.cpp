// This is our program entry point. From here, we generate the maze, solve it, and then display the results.
// We have our GUI thing, then we should be able to swap in any algorithm generator or solver

/*
How to use:
    - Edit the constants in src/constants.cpp to your liking
    - Execute
*/
#include <ctime>
#include <stdexcept>
#include "../lib/raylib.h"
#include "constants.cpp"
#include "generators/ellers.h"
#include "generators/recursive_backtracking.h"
#include "solvers/naive_recursive_solver.h"
#include "solvers/weighted_proximity_recursive.h"
#include "utils.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include <iostream>
using namespace constants;

int main() {
    srand(time(NULL));
    // Create an empty data structure to hold the future maze
    gridType grid = createEmptyGrid(ROWS, COLS);
    auto dims = utils::calculateCanvasDimensions();

    // Populate the empty data structure, to turn it into a maze. Effectively, this replaces
    // the 0's in the data structure with other numbers. Those numbers indicate which sides 
    // of the cell in that location are unblocked by walls (and are therefore valid connections).
    // For example a value equalling NORTH+SOUTH (refer to the file containing algorithm constants),
    // indicates that only the cells to the north and south of the current cell are unblocked 
    // by walls. They are therefore valid connections for the current cell. 
    switch (currentGenerator) {
        case RECURSIVE_BACKTRACKING: {
            // TODO: get WASM display working. The desktop version is now fine.
            InitWindow(dims.x, dims.y, "Maze Generator: recursive backtracking");
            rb::_nonWasmFuncToDisplayMazeBuildSteps(&grid);
            break;
        }
        case ELLERS:
            // el::generateMazeInstantlyNoDisplay();
            grid = el::exportCardinalMaze();
            // TODO: WIP replace these lines. At the moment, we use them to print the final state of the maze
            InitWindow(dims.x, dims.y, "Maze Generator: Eller's algorithm");
            el::_nonWasmFuncToDisplayMazeBuildSteps(&grid);
            std::cout << "Generated maze using Eller's algorithm\n" << std::endl;
            break;

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

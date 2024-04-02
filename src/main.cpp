// This is our program entry point. From here, we generate the maze, solve it, and then display the results.
// We have our GUI thing, then we should be able to swap in any algorithm generator or solver

/*
How to use:
    - Specify your preferred algorithms below
    - Edit the constants in src/constants.cpp to change the size of the maze, the speed of the generation and solving
    - Execute
*/
#include <ctime>
#include <stdexcept>
#include "constants.cpp"
#include "generators/recursive_backtracking.h"
#include "solvers/naive_recursive_solver.h"

using namespace constants;

// Choose one of the available algorithms to generate the maze
enum generatorAlgorithm { RECURSIVE_BACKTRACKING, SKIP_GENERATION };
generatorAlgorithm currentGenerator = RECURSIVE_BACKTRACKING;

// Choose one of the available algorithms to solve the maze
enum solverAlgorithm { NAIVE_RECURSIVE, SKIP_SOLVING };
solverAlgorithm currentSolver = NAIVE_RECURSIVE;

// Set the start and end points for the solving algorithm. These values should be 0 indexed
utils::XY solverStart = {0, 0};
utils::XY solverEnd = {ROWS - 1, COLS - 1};

int main() {
    srand(time(NULL));

    gridType grid = generateGrid(ROWS, COLS);
    switch (currentGenerator) {
        case RECURSIVE_BACKTRACKING:
            srand(time(NULL));
            RB::recursive_backtracking rb;
            // Enable one of the following lines only.
            // rb.generateMazeInstantlyNoDisplay(&grid);
            rb.displayMazeBuildSteps(&grid);
            break;

        case SKIP_GENERATION:
            break;

        default:
            // throw a not yet implemented exception
            throw std::invalid_argument("The chosen generator algorithm is not yet implemented");
    };

    switch (currentSolver) {
        case NAIVE_RECURSIVE:
            ns::naiveSolver(grid, solverStart, solverEnd);
            ns::animateSolution(grid);
            break;

        case SKIP_SOLVING:
            break;

        default:
            // throw a not yet implemented exception
            throw std::invalid_argument("The chosen solver algorithm is not yet implemented");
    }
}
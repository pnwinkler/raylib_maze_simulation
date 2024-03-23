// Given a grid, find a contiguous line between the defined start point and end point
// This is a naive solver.

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include "../constants.cpp"
#include "../generators/recursive_backtracking.cpp"
#include "../utils.cpp"
#include "deque"
#include "unordered_set"

struct XY {
    int x;
    int y;
};

std::deque<XY> myStack = {};
std::unordered_set<int> indicesChecked = {};
std::deque<XY> locationsInOrderChecked = {};
XY head;

std::vector<XY> naiveSolver(gridType& grid, XY startLoc, XY endLoc) {
    // Given a valid maze, return a path within that maze, connecting the start and end locations,
    // while respecting maze walls
    if (endLoc.x >= grid.at(0).size() || endLoc.y >= grid.size()) {
        throw std::invalid_argument("Target location out of grid bounds");
    }

    bool found = false;
    std::vector<XY> solution = {};
    myStack.push_front(startLoc);
    bool nextStep(gridType & grid, XY target);
    while (indicesChecked.size() < (grid.size() * grid.at(0).size()) || !found) {
        found = nextStep(grid, endLoc);
    }

    if (!found) {
        std::cout << "Failed to find solution connecting points (" << startLoc.y << "," << startLoc.x << ") and ("
                  << endLoc.x << "," << endLoc.y << ")";
        // todo: if this doesn't compile, replace it with an empty XY vector
        return {};
    }
    return solution;
}

bool nextStep(gridType& grid, XY target) {
    // Perform the next step of the naive algorithm. Return true if target was found, else false
    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    XY currentLocation = myStack.front();
    myStack.pop_front();

    for (const auto& direction : directions) {
        if (currentLocation.y == target.y && currentLocation.x == target.x) {
            std::cout << "FOUND at " << currentLocation.x << ',' << currentLocation.y << '\n';
            return true;
        }

        // TODO: respect walls
        XY nextLoc = {currentLocation.x + DX[direction], currentLocation.y + DY[direction]};
        if (nextLoc.x < grid.at(0).size() && nextLoc.y < grid.size() &&
            !indicesChecked.contains((nextLoc.y * grid.size()) + nextLoc.x)) {
            // todo: set up clang style to not have the conditional at the same indentation as its block
            indicesChecked.insert((nextLoc.y * grid.size()) + nextLoc.x);
            myStack.push_back(nextLoc);

            bool directionBlockedByWall;  // todo

            std::cout << "Checking location " << nextLoc.x << "," << nextLoc.y << '\n';

            auto origVal = grid.at(nextLoc.y)[nextLoc.x];
            grid.at(nextLoc.y)[nextLoc.x] = -1;  // TEMP placeholder value, until we can print everything nicely
            // TODO; instead of printing state here, create a data structure which our printing function will be able to
            // use later. The idea is, we run through full simulation, then print results at our leisure.
            locationsInOrderChecked.push_back(nextLoc);
            grid.at(nextLoc.y)[nextLoc.x] = origVal;
        }
    }
    return false;
}

void animateSolution(gridType& grid) {
    InitWindow(xPixels, yPixels, "Maze");
    // todo: set up headers etc so that we don't see the _simulationDraw function as being available in this file

    void _solverDraw(gridType & grid, XY & checkedLocation);
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (locationsInOrderChecked.size() > 0) {
            XY checkedLocation = locationsInOrderChecked.front();
            locationsInOrderChecked.pop_front();
            std::cout << checkedLocation.x << checkedLocation.y << '\n';
            _solverDraw(grid, checkedLocation);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        EndDrawing();
    }
    CloseWindow();
}

void _solverDraw(gridType& grid, XY& checkedLocation) {
    // Helper function, to draw grid state in GUI. Expects an existing window.

    // TODO: figure out why checkedLocation values are always 0,0 here, but not in the func above?
    std::cout << checkedLocation.x << checkedLocation.y << '\n';
    ClearBackground(RAYWHITE);
    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            int val = grid.at(y).at(x);

            if (val != SOUTH && !(y < grid.size() - 1 && grid.at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * cellWidth, (y + 1) * cellHeight, (x + 1) * cellWidth, (y + 1) * cellHeight, BLACK);
            if (val != EAST && !(x < grid.at(0).size() - 1 && grid.at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * cellWidth, y * cellHeight, (x + 1) * cellWidth, (y + 1) * cellHeight, BLACK);

            DrawRectangle(checkedLocation.x * cellWidth, checkedLocation.y * cellHeight, cellWidth, cellHeight, PINK);
        }
    }
}

int main() {
    // TODO: implement basic solver
    //  - print each step
    //  - respect walls
    //  - test with larger scale mazes
    // TODO: confirm maze solver works for properly generated maze and it respects walls
    // TODO: get graphical display of solution working
    srand(time(NULL));
    int rows = 3, cols = 3;
    indicesChecked.reserve(rows * cols);
    gridType grid = generateGrid(rows, cols);

    // instantly generate the maze
    generateMazeInstantly(&grid);
    // displayMazeIterations(&grid);

    // XY startLoc = {0, 0};
    // XY endLoc = {0, 0};
    // naiveSolver(&grid, startLoc, endLoc);

    XY start = {0, 0};
    XY end = {2, 2};
    naiveSolver(grid, start, end);

    animateSolution(grid);
}

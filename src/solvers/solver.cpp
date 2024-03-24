// Given a grid, find a contiguous line between the defined start point and end point
// This is a naive solver.

#include <raylib.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>
#include "../constants.cpp"
#include "../generators/recursive_backtracking.cpp"
#include "../utils.cpp"
#include "deque"
#include "unordered_set"

std::unordered_set<int> indicesChecked = {};
std::deque<XY> locationsInOrderVisited = {};

void naiveSolver(gridType& grid, XY startLoc, XY endLoc) {
    // Given a valid maze, return a path within that maze, connecting the start and end locations,
    // while respecting maze walls
    if (endLoc.x >= grid.at(0).size() || endLoc.y >= grid.size()) {
        throw std::invalid_argument("Target location out of grid bounds");
    }
    std::deque<XY> locationsToCheck = {};

    bool found = false;
    locationsToCheck.push_front(startLoc);
    indicesChecked.insert(startLoc.y * grid.size() + startLoc.x);

    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            std::cout << grid[y][x];
        }
        std::cout << '\n';
    }

    // Forward declaration
    bool nextStep(gridType & grid, XY target, std::deque<XY> & locationsToCheck);
    while (!found && (indicesChecked.size() < grid.size() * grid.at(0).size()) && locationsToCheck.size() > 0) {
        std::cout << "DEBUG: LOCATIONSTOCHECK=";
        for (XY thing : locationsToCheck) {
            std::cout << "(" << thing.x << "," << thing.y << "), ";
        }
        std::cout << '\n';

        found = nextStep(grid, endLoc, locationsToCheck);
    }

    if (!found) {
        std::cout << "Failed to find solution connecting points (" << startLoc.y << "," << startLoc.x << ") and ("
                  << endLoc.x << "," << endLoc.y << ")" << std::endl;
        return;
    }
    return;
}

bool nextStep(gridType& grid, XY target, std::deque<XY>& locationsToCheck) {
    // Perform the next step of the naive algorithm. Return true if target was found, else false
    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    XY currentLocation = locationsToCheck.front();
    locationsToCheck.pop_front();
    locationsInOrderVisited.push_back(currentLocation);
    indicesChecked.insert((currentLocation.y * grid.size()) + currentLocation.x);

    assert(inBounds(grid, currentLocation));
    // assert(currentLocation.x < grid.at(0).size() && currentLocation.y < grid.size());

    if (currentLocation.y == target.y && currentLocation.x == target.x) {
        std::cout << "FOUND at " << currentLocation.x << ',' << currentLocation.y << '\n';
        return true;
    }

    // TODO: also check if ANY of the neighboring cells point in this cell's direction
    //  reason: a cell can have 2 sides without a wall, but can only point in 1 direction at a time
    //  therefore, if our current cell does not point to a 2nd non-wall direction, then the cell in that
    //  location ought to point to our current cell
    //    2 cells can point to each other

    for (auto direction : directions) {
        XY targetedCell = {currentLocation.x + DX[direction], currentLocation.y + DY[direction]};
        bool targetInBounds = inBounds(grid, targetedCell);
        // this can go out of bounds
        bool indexChecked = indicesChecked.contains((targetedCell.y * grid.size()) + targetedCell.x);
        // either our cell points to that cell or that cell points to our cell
        bool noWallBetween = targetInBounds && (grid[currentLocation.y][currentLocation.x] == direction ||
                                                grid[targetedCell.y][targetedCell.x] == OPPOSITE[direction]);

        std::cout << "DEBUG: location " << targetedCell.x << "," << targetedCell.y
                  << " targetInBounds=" << targetInBounds << " indexChecked=" << indexChecked << '\n';
        if (!indexChecked && noWallBetween) {
            // todo? set up clang style to not have the conditional at the same indentation as its block
            locationsToCheck.push_back(targetedCell);
            // std::cout << "DEBUG: PUSHING BACK with " << nextLoc.y << ',' << nextLoc.x << ". Size of stack is now "
            // << locationsToCheck.size() << std::endl;
            // std::cout<<"DEBUG: ADDING LOCATION " << nextLoc.y << nextLoc.x<< '\n';
        }
    }

    return false;
}

void animateSolution(gridType& grid) {
    // todo: set up headers etc so that we don't see the _simulationDraw function as being available in this file

    // Forward declaration
    void _solverDraw(gridType & grid, XY & checkedLocation);

    InitWindow(xPixels, yPixels, "Maze");
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (locationsInOrderVisited.size() > 0) {
            XY checkedLocation = locationsInOrderVisited.front();
            locationsInOrderVisited.pop_front();
            // std::cout << "DEBUG: " << checkedLocation.x << checkedLocation.y << '\n';
            _solverDraw(grid, checkedLocation);
        }
        EndDrawing();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    CloseWindow();
}

void _solverDraw(gridType& grid, XY& checkedLocation) {
    // Helper function, to draw grid state in GUI. Expects an existing window.

    // std::cout << "DEBUG: Checked location " << checkedLocation.x << checkedLocation.y << '\n';
    DrawText("Naive solver", 5, 5, 6, RED);
    ClearBackground(RAYWHITE);
    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            // the offsets are intended to stop this shape from being drawn over the walls of the maze
            auto mazeEndpoint = locationsInOrderVisited.back();
            DrawRectangle(mazeEndpoint.x * cellWidth, mazeEndpoint.y * cellHeight + 1, cellWidth - 1,
                          cellHeight - 1, LIGHTGRAY);
            DrawRectangle(checkedLocation.x * cellWidth, checkedLocation.y * cellHeight + 1, cellWidth - 1,
                          cellHeight - 1, PINK);

            int val = grid.at(y).at(x);
            if (val != SOUTH && !(y < grid.size() - 1 && grid.at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * cellWidth, (y + 1) * cellHeight, (x + 1) * cellWidth, (y + 1) * cellHeight, BLACK);
            if (val != EAST && !(x < grid.at(0).size() - 1 && grid.at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * cellWidth, y * cellHeight, (x + 1) * cellWidth, (y + 1) * cellHeight, BLACK);
        }
    }
}

int main() {
    // TODO: make display nicer
    //  - indicate the end point
    //  - maybe indicate the start point
    //  - optionally (?) trace the path, e.g. by having fading pink squares
    srand(time(NULL));
    int rows = 6, cols = 6;
    indicesChecked.reserve(rows * cols);
    gridType grid = generateGrid(rows, cols);

    // instantly generate the maze
    generateMazeInstantly(&grid);
    // displayMazeIterations(&grid);

    // XY startLoc = {0, 0};
    // XY endLoc = {0, 0};
    // naiveSolver(&grid, startLoc, endLoc);

    // these should be 0 indexed
    XY start = {1, 1};
    XY end = {5, 5};
    naiveSolver(grid, start, end);

    animateSolution(grid);
}

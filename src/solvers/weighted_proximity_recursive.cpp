// Given a grid, find a contiguous line between the defined start point and end point
// This algorithm recursively checks each cell, to see if one of its neighbors is the maze end cell. It never visits a
// cell twice. It uses a proximity weighting function to prioritize each possible move.

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
#include "vector"

struct scoreGroup {
    int score;
    std::vector<XY> *cells;
};
typedef std::vector<scoreGroup> sortedScores;
sortedScores allScores = {};

std::unordered_set<int> indicesChecked = {};
std::deque<XY> locationsInOrderVisited = {};

int calculateWeight(XY cell, XY mazeFinish) {
    int diffX = mazeFinish.x - cell.x;
    int diffY = mazeFinish.y - cell.y;
    if (diffX < 0) {
        diffX *= -1;
    }
    if (diffY < 0) {
        diffY *= -1;
    }
    return diffX + diffY;
}

void insertIntoScores(XY cell, int score) {
    // If a relevant group already exists, then extend it if the XY coordinate has is not part of the set.

    // std::cout << "DEBUG: size=" << allScores.size() << '\n';
    for (int x = allScores.size() - 1; x > -1; x--) {
        if (score > allScores[x].score) {
            break;
        } else if (score == allScores[x].score) {
            // add only if an equivalent XY coordinate is not already there
            std::cout<<"DEBUG: score group already exists\n";
            for (int i = 0; i < allScores[x].cells->size(); i++) {
                XY existingCell = allScores[x].cells->at(i);
                if (existingCell.y == cell.y && existingCell.y == cell.y) {
                    std::cout<<"DEBUG: cell ("<<cell.x<<','<<cell.y<<") already exists in score group as (" << existingCell.x<<','<<existingCell.y<<")\n";
                    return;
                }
                std::cout<<"HAPPENING 2. Adding ("<<cell.x<<','<<cell.y<<") to allScores["<<x<<"]\n";
                allScores[x].cells->push_back(cell);
            }
            return;
        }
    }

    // Either the new score does not equal any other, or allScores has 0 elements.
    // std::cout<<"DEBUG: Adding (" << cell.x<<','<<cell.y<<") to new score group\n";
    std::vector<XY> cells = {cell};
    scoreGroup sg = {score, &cells};
    allScores.push_back(sg);
}

bool nextStep(gridType& grid, XY target) {
    // Perform the next step of the algorithm. Return true if target was found, else false.

    // select any of the elements with the greatest weight
    auto set = allScores[0]->cells;
    std::cout<<"DEBUG 1: set->size()="<<set->size()<<'\n';
    XY currentBest = *set->begin();
    set->erase(set->begin());
    std::cout<<"DEBUG 2: set->size()="<<set->size()<<'\n';

    if (set->size() == 0) {
        allScores.erase(allScores.begin());
    }
    std::cout<<"DEBUG 3: set.size()="<<set->size()<<'\n';

    assert(inBounds(grid, currentBest));

    locationsInOrderVisited.push_back(currentBest);
    indicesChecked.insert((currentBest.y * grid.size()) + currentBest.x);

    if (currentBest.y == target.y && currentBest.x == target.x) {
        std::cout << "FOUND at " << currentBest.x << ',' << currentBest.y << '\n';
        return true;
    }

    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    int placeholder = 9999;
    int minWeight = placeholder;
    XY minNeighbor;
    for (auto direction : directions) {
        XY neighbor = {currentBest.x + DX[direction], currentBest.y + DY[direction]};
        bool targetInBounds = inBounds(grid, neighbor);
        bool indexChecked = indicesChecked.contains((neighbor.y * grid.size()) + neighbor.x);
        // Either our cell points to that cell or that cell points to our cell
        bool noWallBetween = targetInBounds && (grid[currentBest.y][currentBest.x] == direction ||
                                                grid[neighbor.y][neighbor.x] == OPPOSITE[direction]);

        std::cout << "DEBUG: HAPPENING 1. Currentloc=(" << currentBest.x<<','<<currentBest.y<<"), bools: (" << indexChecked << ',' << noWallBetween << ") for (" << neighbor.x << ','
        << neighbor.y << ')' << '\n';
        // TODO: verify if we need min weight and min neighbor here?
        if (!indexChecked && noWallBetween) {
            // std::cout << "DEBUG: HAPPENING 2" << '\n';
            int weight = calculateWeight(neighbor, target);
            std::cout << "DEBUG: score=" << weight << " for cell at (" << neighbor.x << ',' << neighbor.y << ')'
                      << '\n';
            insertIntoScores(neighbor, weight);
            // std::cout<<"DEBUG 1: allScores.size()="<<allScores.size()<<'\n';
            // std::cout<<"DEBUG 2: allScores.size()="<<allScores.size()<<'\n';
        }
    }

    // TODO: resolve why the element isn't being removed
    // std::cout<<"DEBUG 1: allScores.size()="<<allScores.size()<<'\n';

    return false;
}

void proximitySolver(gridType& grid, XY startLoc, XY endLoc) {
    // Given a valid maze, find a path within that maze, connecting the start and end locations,
    // while respecting maze walls.
    if (endLoc.x >= grid.at(0).size() || endLoc.y >= grid.size()) {
        throw std::invalid_argument("Target location out of grid bounds");
    }

    bool found = false;
    auto weight = calculateWeight(startLoc, endLoc);
    insertIntoScores(startLoc, weight);
    indicesChecked.insert(startLoc.y * grid.size() + startLoc.x);

    std::cout << allScores.size() << '\n';
    while (!found && (indicesChecked.size() < grid.size() * grid.at(0).size()) && allScores.size() > 0) {
        // std::cout << "DEBUG: HAPPENING 1" << '\n';
        found = nextStep(grid, endLoc);
    }

    if (!found) {
        std::cout << "DEBUG: found=" << found << " indicesCheckedSize=" << indicesChecked.size()
                  << " allScores.size()= " << allScores.size() << '\n';
        std::cerr << "Failed to find solution connecting points (" << startLoc.y << "," << startLoc.x << ") and ("
                  << endLoc.x << "," << endLoc.y << ")" << std::endl;
    }
}

void animateSolution(gridType& grid) {
    // todo: set up headers etc so that we don't see the _simulationDraw function as being available in this file

    // Forward declaration
    void _solverDraw(gridType & grid, int locationIdx);
    auto dims = calculateCanvasDimensions();
    SetTargetFPS(FPS_SOLVING);

    InitWindow(dims.x, dims.y, "Maze solving: naive recursion");
    int locationIndex = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (locationIndex < locationsInOrderVisited.size()) {
            _solverDraw(grid, locationIndex);
            locationIndex++;
        }
        EndDrawing();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS_SOLVING));
    }
    CloseWindow();
}

Color gradateColor(Color start, Color target, int i, int locationIdx) {
    // Apply a function to return a color between the start and target colors
    auto d1 = target.r - start.r;
    auto d2 = target.g - start.g;
    auto d3 = target.b - start.b;
    auto d4 = target.a - start.a;

    // For i values close to locationIdx, return more similar colors than for i values further from it.
    // Reasoning: I believe it's visually more appealing (and less distracting) to use fairly uniform colors for cells
    // visited long ago. This uniformity also effectively exaggerates the constrast between recently visited cells,
    // making it easier at a glance to reconstruct the recent path taken.
    float multiplier;
    if (locationIdx - i < 3) {
        // Gradate normally
        multiplier = (float)i / locationIdx;
    } else {
        multiplier = (float)i / (locationIdx * 1.5);
    }

    int d1Col = target.r - (d1 * (multiplier));
    int d2Col = target.g - (d2 * (multiplier));
    int d3Col = target.b - (d3 * (multiplier));
    int d4Col = target.a - (d4 * (multiplier));
    Color clr = Color(d1Col, d2Col, d3Col, d4Col);
    return clr;
}

void _solverDraw(gridType& grid, int locationIdx) {
    // Helper function, to draw grid state in GUI. Expects an existing window.

    ClearBackground(RAYWHITE);

    auto checkedLocation = locationsInOrderVisited.at(locationIdx);

    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            // The offsets are intended to stop this shape from being drawn over the walls of the maze
            auto mazeEndpoint = locationsInOrderVisited.back();
            DrawRectangle(mazeEndpoint.x * CELLWIDTH, mazeEndpoint.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                          LIGHTGRAY);
            DrawRectangle(checkedLocation.x * CELLWIDTH, checkedLocation.y * CELLHEIGHT + 1, CELLWIDTH - 1,
                          CELLHEIGHT - 1, PURPLE);

            // Add indication of previously visited cells
            for (int i = 0; i < locationIdx; i++) {
                Color clr = gradateColor(PURPLE, RAYWHITE, i, locationIdx);
                auto visitedLoc = locationsInOrderVisited.at(i);
                DrawRectangle(visitedLoc.x * CELLWIDTH, visitedLoc.y * CELLHEIGHT + 1, CELLWIDTH - 1, CELLHEIGHT - 1,
                              clr);
            }
            DrawText("Naive solver", 5, 5, 6, RED);

            // Draw the walls
            int val = grid.at(y).at(x);
            if (val != SOUTH && !(y < grid.size() - 1 && grid.at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            if (val != EAST && !(x < grid.at(0).size() - 1 && grid.at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
        }
    }
}

int main() {
    // TODO:
    //  improve the graphical display by:
    //    consider adding stats, like % cells visited, steps performed, dead ends encountered, etc
    //  make a proximity based recursive solver once done with this solver
    // TODO: see if we can "statically link" (or whatever it's called) the relevant parts of raylib when compiling
    srand(time(NULL));
    indicesChecked.reserve(ROWS * COLS);
    gridType grid = generateGrid(ROWS, COLS);

    // Enable one of the following lines only. One shows the maze-to-be-solved being built, whereas the other builds it
    // but doesn't show it
    // displayMazeBuildSteps(&grid);
    generateMazeInstantly(&grid);

    // these should be 0 indexed
    XY start = {0, 0};
    XY end = {ROWS - 1, COLS - 1};
    std::cout << "DEBUG 1: " << ROWS - 1 << COLS - 1 << std::endl;
    std::cout << "DEBUG 2: " << end.x << end.y << std::endl;
    proximitySolver(grid, start, end);

    animateSolution(grid);
}

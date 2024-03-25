/*
    **WARNING**: on large mazes, this program may CRASH your computer!
     That's probably due to a stack overflow from the recursive calls (in my case it crashed
     when there were ~1320 tasks in queue when operating on 1920 by 1080px with cell width
     and height both set to 20)
*/
// see https://zig.news/perky/hot-reloading-with-raylib-4bf9 for interesting ideas

// Generate a maze using the recursive backtracking algorithm and display it in the console
#include <raylib.h>
#include <deque>
#include <functional>  // for std::bind
#include <future>
#include <iostream>
#include <random>
#include "../constants.cpp"
#include "../utils.cpp"
// TODO: re-enable this once done with solvers/solver.cpp stuff
// #include "./constants.cpp"
// #include "./utils.cpp"

//------------------------------------------------------------------------------
// Set up data structures to describe passage direction and aid with maze creation
//------------------------------------------------------------------------------

// The dimensions of each cell that we'll create. The cellWidth and cellHeight should
// divide into xPixels and yPixels respectively, with 0 remainder.
constexpr int cellWidth = 40;
constexpr int cellHeight = 40;

// Did you read the warning above?
constexpr int xPixels = 800;
constexpr int yPixels = 800;
constexpr int fps = 10;

// hack: used to indicate whether it's the first tick of our simulation
bool firstSimulationTick = true;

//------------------------------------------------------------------------------
// Functions for displaying the maze
//------------------------------------------------------------------------------

// Holds the X and Y locations of the most recent edit on the grid
// Each edit can affect 0-2 locations inclusive.
struct mostRecentGridEdit {
    int x0;
    int x1;
    int y0;
    int y1;
};
struct mostRecentGridEdit mrge;

// Holds tasks required for simulation, in a queue, to be called later
std::deque<std::packaged_task<bool()>> taskDeque;

void simulationTick(gridType* grid) {
    // Forward declarations
    bool _carvePassagesFrom(int startX, int startY, gridType* grid);
    // void _displayMazeInConsole(gridType * grid);

    // The changeEffected variable is used to indicate whether a change to the grid's state has occurred.
    // I think it's better a better user experience if we fast forward over tasks that don't change grid state.
    // This variable lets us do that. It's because of our queueing system, that our task queue sometimes
    // contains tasks which don't change the grid's state.
    auto changeEffected = false;

    if (firstSimulationTick) {
        firstSimulationTick = false;
        // _displayMazeInConsole(grid);
        _carvePassagesFrom(0, 0, grid);
        return;
    }

    while (!changeEffected && !taskDeque.empty()) {
        std::packaged_task<bool()> task = std::move(taskDeque.front());
        taskDeque.pop_front();
        auto fut = task.get_future();
        task();
        changeEffected = fut.get();

        // All tasks have finished running. We do one last display update,
        // to update the task count in the graphical window
        // if (taskDeque.empty()) {
        // _simulationDraw(grid);
        // }
    }

    // We only need to update the display if the grid's state has changed
    // if (changeEffected) {
    //     // _simulationDraw(grid);
    //     _displayMazeInConsole(grid);
    // }
}

void displayMazeBuildSteps(gridType* grid) {
    // Displays the state of the maze in the graphical window, progressing one simulation tick
    // per frame displayed
    InitWindow(xPixels, yPixels, "Maze");
    SetTargetFPS(fps);

    void _simulationDraw(gridType * grid);
    while (!WindowShouldClose()) {
        BeginDrawing();
        _simulationDraw(grid);
        // todo: set this up, so that I can simultaneously update the graphical display as well
        // as the console printout
        simulationTick(grid);
        EndDrawing();
    }
    CloseWindow();
}

void generateMazeInstantly(gridType* grid) {
    do {
        simulationTick(grid);
    } while (!taskDeque.empty());
}

void _simulationDraw(gridType* grid) {
    // Helper function, to draw grid state in GUI. Expects an existing window.

    ClearBackground(RAYWHITE);
    DrawText(("Tasks " + std::to_string(taskDeque.size())).c_str(), 10, 10, 10, MAROON);
    for (int y = 0; y < grid->size(); y++) {
        for (int x = 0; x < grid->at(0).size(); x++) {
            int val = grid->at(y).at(x);

            if (val != SOUTH && !(y < grid->size() - 1 && grid->at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * cellWidth, (y + 1) * cellHeight, (x + 1) * cellWidth, (y + 1) * cellHeight, BLACK);
            if (val != EAST && !(x < grid->at(0).size() - 1 && grid->at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * cellWidth, y * cellHeight, (x + 1) * cellWidth, (y + 1) * cellHeight, BLACK);

            if (mrge.x0 == x && mrge.y0 == y)
                DrawRectangle(mrge.x0 * cellWidth, mrge.y0 * cellHeight, cellWidth, cellHeight, PINK);
            if (mrge.x1 == x && mrge.y1 == y)
                DrawRectangle(mrge.x1 * cellWidth, mrge.y1 * cellHeight, cellWidth, cellHeight, PINK);
        }
    }
}

//------------------------------------------------------------------------------
// The algorithm itself
//------------------------------------------------------------------------------

bool _carvePassagesFrom(int startX, int startY, gridType* grid) {
    // Connects two cells in the grid, subject to constraints.
    // Intended to be called indirectly.

    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(directions.begin(), directions.end(), g);
    for (const auto& direction : directions) {
        int newX = startX + DX[direction];
        int newY = startY + DY[direction];

        bool targetInBounds = newY >= 0 && newY < (int)grid->size() && newX >= 0 && newX < (int)grid->at(0).size();
        if (targetInBounds && grid->at(newY).at(newX) == 0) {
            // Queueing tasks lets us more easily control the interval between simulation
            // steps, which makes rendering the state easier

            // bool carvingHelper(int startX, int startY, int newX, int newY, int direction, gridType* grid);
            // these 2 lines each give identical error messages on compile time. Leaving the forward decl above or
            // removing it also has no impact on the error messages std::packaged_task<bool()> task([this, startX,
            // startY, newX, newY, direction, &grid] mutable {carvingHelper(startX, startY, newX, newY, direction,
            // grid);}); std::packaged_task<bool()> task([this, startX, startY, newX, newY, direction, &grid] mutable
            // {this->carvingHelper(startX, startY, newX, newY, direction, grid);});
            // taskDeque.push_front(std::move(task));

            // std::packaged_task<bool(int, int, int, int, int, gridType*)> x( carvingHelper(startX, startY, newX, newY,
            // direction, grid));

            bool carvingHelper(int startX, int startY, int newX, int newY, int direction, gridType* grid);
            std::packaged_task<bool()> task(std::bind(carvingHelper, startX, startY, newX, newY, direction, grid));
            taskDeque.push_front(std::move(task));
        }
    }
    // For our task queue, false indicates that no grid state changes were made
    return false;
}

bool carvingHelper(int startX, int startY, int newX, int newY, int direction, gridType* grid) {
    // Connect source and target cells. Returns true if it changed something, else false.
    // Having this as a separate function allows us to have one pop of the task queue correspond
    // to 0-1 updates of the board's state.

    // auto DEBUG_1 = grid->at(startY).at(startX);
    // auto DEBUG_2 = grid->at(newY).at(newX);
    bool targetInBounds = newY >= 0 && newY < (int)grid->size() && newX >= 0 && newX < (int)grid->at(0).size();
    bool cond = targetInBounds && grid->at(newY).at(newX) == 0;

    if (cond) {
        mrge.y0 = -1;
        mrge.x0 = -1;
        mrge.x1 = -1;
        mrge.y1 = -1;

        // Don't overwrite existing connections
        if (grid->at(startY).at(startX) == 0) {
            grid->at(startY).at(startX) = direction;
            mrge.x0 = startX;
            mrge.y0 = startY;
            // std::cout << "Debug: changed start (" << startY << "," << startX << ") from " << DEBUG_1 << " to " <<
            // direction << '\n';
        }
        grid->at(newY).at(newX) = OPPOSITE[direction];
        mrge.x1 = newX;
        mrge.y1 = newY;
        // std::cout << "Debug: changed target (" << newY << "," << newX << ") from " << DEBUG_2 << " to " <<
        // OPPOSITE[direction] << std::endl;
    }
    // auto x = [&](int startX, int startY, gridType* grid){return _carvePassagesFrom(newX, newY, grid);};
    std::packaged_task<bool()> task(std::bind(_carvePassagesFrom, newX, newY, grid));
    taskDeque.push_front(std::move(task));
    return cond;
}

// TODO: add header file (with guard(s)), and set this up to work with that
// then fix imports!

// int main() {
//     if (xPixels % cellWidth != 0 || yPixels % cellHeight != 0) {
//         std::cout << "Pixel dimensions (" << xPixels << ", " << yPixels
//                   << ") cannot be neatly divided by cell dimensions."
//                   << " This may result in a maze that doesn't neatly fit the screen\n";
//     }
//
//     srand(time(NULL));
//     gridType grid = generateGrid((int)xPixels / cellWidth, (int)yPixels / cellHeight);
//
//     generateMazeInstantly(&grid);
//     displayMazeIterations(&grid);
//
//     return EXIT_SUCCESS;
// }

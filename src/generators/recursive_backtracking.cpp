// Generate a maze using the recursive backtracking algorithm and display it graphically

#include <raylib.h>
#include <deque>
#include <functional>  // for std::bind
#include <future>
#include <random>
#include "../constants.cpp"
#include "../utils.cpp"

//------------------------------------------------------------------------------
// Set up data structures to describe passage direction and aid with maze creation
//------------------------------------------------------------------------------

// hack: used to indicate whether it's the first tick of our simulation
bool firstSimulationTick = true;

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
    // Progress the state of the maze generation by one tick.

    // Forward declaration
    bool _carvePassagesFrom(XY & start, gridType * grid);

    if (firstSimulationTick) {
        firstSimulationTick = false;
        XY start = {0, 0};
        _carvePassagesFrom(start, grid);
        return;
    }

    // The changeEffected variable is used to indicate whether a change to the grid's state has occurred.
    // It lets us fast forward over tasks that don't change grid state (which is a better user experience IMO).
    // Recursive backtracking inherently performs ticks that don't change maze state.
    auto changeEffected = false;

    while (!changeEffected && !taskDeque.empty()) {
        std::packaged_task<bool()> task = std::move(taskDeque.front());
        taskDeque.pop_front();
        auto fut = task.get_future();
        task();
        changeEffected = fut.get();
    }
}

void displayMazeBuildSteps(gridType* grid) {
    // Displays the state of the maze in the graphical window, progressing one simulation tick
    // per frame displayed
    auto dims = calculateCanvasDimensions();
    InitWindow(dims.x, dims.y, "Maze generation: recursive backtracking");
    SetTargetFPS(FPS_GENERATING);

    // Forward declaration
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
    // Helps draw grid state in GUI. Expects an existing window.

    ClearBackground(RAYWHITE);
    DrawText(("Tasks " + std::to_string(taskDeque.size())).c_str(), 10, 10, 10, MAROON);
    for (int y = 0; y < grid->size(); y++) {
        for (int x = 0; x < grid->at(0).size(); x++) {
            int val = grid->at(y).at(x);

            if (val != SOUTH && !(y < grid->size() - 1 && grid->at(y + DY[SOUTH])[x] == NORTH))
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            if (val != EAST && !(x < grid->at(0).size() - 1 && grid->at(y)[x + DX[EAST]] == WEST))
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);

            if (mrge.x0 == x && mrge.y0 == y)
                DrawRectangle(mrge.x0 * CELLWIDTH, mrge.y0 * CELLHEIGHT, CELLWIDTH, CELLHEIGHT, PINK);
            if (mrge.x1 == x && mrge.y1 == y)
                DrawRectangle(mrge.x1 * CELLWIDTH, mrge.y1 * CELLHEIGHT, CELLWIDTH, CELLHEIGHT, PINK);
        }
    }
}

//------------------------------------------------------------------------------
// The algorithm itself
//------------------------------------------------------------------------------

bool _carvePassagesFrom(XY& start, gridType* grid) {
    // Connects two cells in the grid, subject to constraints.

    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    for (const auto& direction : directions) {
        XY neighbor = {start.x + DX[direction], start.y + DY[direction]};
        bool targetInBounds = inBounds(*grid, neighbor);

        if (targetInBounds && grid->at(neighbor.y).at(neighbor.x) == 0) {
            // Queueing tasks lets us more easily control the interval between simulation
            // steps, which makes rendering the state easier

            // Forward declaration
            bool carvingHelper(XY & start, XY & target, int direction, gridType* grid);
            std::packaged_task<bool()> task(std::bind(carvingHelper, start, neighbor, direction, grid));
            taskDeque.push_front(std::move(task));
        }
    }
    // For our task queue, false indicates that no grid state changes were made
    return false;
}

bool carvingHelper(XY& start, XY& target, int direction, gridType* grid) {
    // Connect source and target cells. Returns true if it changed something, else false.
    // Having this as a separate function allows us to have one pop of the task queue correspond
    // to 0-1 updates of the board's state.

    bool targetInBounds = inBounds(*grid, target);
    bool cond = targetInBounds && grid->at(target.y).at(target.x) == 0;

    if (cond) {
        mrge.y0 = -1;
        mrge.x0 = -1;
        mrge.x1 = -1;
        mrge.y1 = -1;

        // Don't overwrite existing connections
        if (grid->at(start.y).at(start.x) == 0) {
            grid->at(start.y).at(start.x) = direction;
            mrge.x0 = start.x;
            mrge.y0 = start.y;
        }
        grid->at(target.y).at(target.x) = OPPOSITE[direction];
        mrge.x1 = target.x;
        mrge.y1 = target.y;
    }

    std::packaged_task<bool()> task(std::bind(_carvePassagesFrom, target, grid));
    taskDeque.push_front(std::move(task));
    return cond;
}

// TODO: add header file (with guard(s)), and set this up to work with that
// then fix imports!

// int main() {
//     if (xPixels % CELLWIDTH != 0 || yPixels % CELLHEIGHT != 0) {
//         std::cout << "Pixel dimensions (" << xPixels << ", " << yPixels
//                   << ") cannot be neatly divided by cell dimensions."
//                   << " This may result in a maze that doesn't neatly fit the screen\n";
//     }
//
//     srand(time(NULL));
//     gridType grid = generateGrid((int)xPixels / CELLWIDTH, (int)yPixels / CELLHEIGHT);
//
//     generateMazeInstantly(&grid);
//     displayMazeIterations(&grid);
//
//     return EXIT_SUCCESS;
// }

/*
    **WARNING**: on large mazes, this program may CRASH your computer!
     That's probably due to a stack overflow from the recursive calls (in my case it crashed
     when there were ~1320 tasks in queue when operating on 1920 by 1080px with cell width
     and height both set to 20)
*/
// see https://zig.news/perky/hot-reloading-with-raylib-4bf9 for interesting ideas

// Generate a maze using the recursive backtracking algorithm and display it in the console
#include <deque>
#include <future>
#include <functional> // for std::bind
#include <iostream>
// #include <random> // for std::random_shuffle
#include <raylib.h>
// #include <thread> // for std::this_thread::sleep_for
// #include <time.h> // for srand
// #include <unordered_set>
// #include <unordered_map>
// #include <vector>

//------------------------------------------------------------------------------
// Set up data structures to describe passage direction and aid with maze creation
//------------------------------------------------------------------------------

// The dimensions of each cell that we'll create, and the canvas's X,Y dimensions
constexpr int cellWidth = 40;
constexpr int cellHeight = 40;
// Did you read the warning above?
constexpr int xPixels = 200;
constexpr int yPixels = 200;
constexpr int fps = 1;

// hack: used to indicate whether it's the first tick of our simulation
bool firstSimulationTick = true;

// Holds the X and Y locations of the most recent edit on the grid
// Each edit can affect 0-2 locations inclusive.
struct mostRecentGridEdit
{
    int x0;
    int x1;
    int y0;
    int y1;
};
struct mostRecentGridEdit mrge;

constexpr int NORTH = 1, SOUTH = 2, EAST = 4, WEST = 8;
// The difference in x for each direction
std::unordered_map<int, int> DX = {
    {NORTH, 0},
    {SOUTH, 0},
    {EAST, 1},
    {WEST, -1}};
std::unordered_map<int, int> DY = {
    {NORTH, -1},
    {SOUTH, 1},
    {EAST, 0},
    {WEST, 0}};
std::unordered_map<int, int> OPPOSITE = {
    {NORTH, SOUTH},
    {SOUTH, NORTH},
    {EAST, WEST},
    {WEST, EAST}};

// Holds tasks required for simulation, in a queue, to be called later
std::deque<std::packaged_task<bool()>> taskDeque;

// Alias
typedef std::vector<std::vector<int>> gridType;

//------------------------------------------------------------------------------
// Functions to assist the algorithm
//------------------------------------------------------------------------------

gridType generateMaze()
{
    // It's better to round down, so that our cells don't go off-screen
    int rowCellCount = xPixels / cellWidth;
    int colCellCount = yPixels / cellHeight;

    if (xPixels % cellWidth != 0 || yPixels % cellHeight != 0)
    {
        std::cout << "Pixel dimensions (" << xPixels << ", " << yPixels << ") cannot be neatly divided by cell dimensions."
                  << " This may result in a maze that doesn't neatly fit the screen\n";
    }

    gridType grid;
    for (int y = 0; y < colCellCount; y++)
    {
        std::vector<int> newRow;
        grid.push_back(newRow);
        for (int x = 0; x < rowCellCount; x++)
        {
            grid[y].push_back(0);
        }
    }
    return grid;
}

//------------------------------------------------------------------------------
// Functions for displaying the maze
//------------------------------------------------------------------------------

void simulationTick(gridType *grid)
{
    // Forward declarations
    bool _carvePassagesFrom(int startX, int startY, gridType *grid);
    void _simulationDraw(gridType * grid);
    void _displayMazeInConsole(gridType * grid);

    // The changeEffected variable is used to indicate whether a change to the grid's state has occurred.
    // I think it's better a better use experience if we fast forward over tasks that don't change grid state.
    // This variable lets us do that. It's because of our queueing system, that our task queue sometimes
    // contains tasks which don't change the grid's state.
    auto changeEffected = false;

    if (firstSimulationTick)
    {
        firstSimulationTick = false;
        _simulationDraw(grid);
        _displayMazeInConsole(grid);
        _carvePassagesFrom(0, 0, grid);
    }
    else
    {
        while (!changeEffected && !taskDeque.empty())
        {
            std::packaged_task<bool()> task = std::move(taskDeque.front());
            taskDeque.pop_front();
            auto fut = task.get_future();
            task();
            changeEffected = fut.get();

            // All tasks have finished running. We do one last display update,
            // to update the task count in the graphical window
            if (taskDeque.empty())
            {
                _simulationDraw(grid);
            }
        }
    }

    // We only need to update the display if the grid's state has changed
    if (changeEffected)
    {
        _simulationDraw(grid);
        _displayMazeInConsole(grid);
    }
}

void displayMaze(gridType *grid)
{
    // Display the maze graphically and in console
    InitWindow(xPixels, yPixels, "Maze");
    SetTargetFPS(fps);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        simulationTick(grid);
        EndDrawing();
    }
    CloseWindow();
}

void _simulationDraw(gridType *grid)
{
    // Helper function, to draw grid state in GUI. Expects an existing window.

    ClearBackground(RAYWHITE);
    DrawText(("Tasks " + std::to_string(taskDeque.size())).c_str(), 10, 10, 10, MAROON);
    for (int y = 0; y < grid->size(); y++)
    {
        for (int x = 0; x < grid->at(0).size(); x++)
        {
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

void _displayMazeInConsole(gridType *grid)
{
    // Helper function. Prints the maze to the console. Can be called directly,
    // but is generally expected to be called indirectly by a function that prompts
    // grid state updates.

    // Create top wall
    int height = grid->size();
    int width = grid->at(0).size();
    for (int i = 0; i < width * 2; i++)
    {
        std::cout << '_';
    }
    std::cout << '\n';

    for (int y = 0; y < height; y++)
    {
        std::cout << '|';
        for (int x = 0; x < width; x++)
        {
            int val = grid->at(y).at(x);
            (val == SOUTH || (y < height - 1 && grid->at(y + DY[SOUTH])[x] == NORTH)) ? std::cout << ' ' : std::cout << '_';
            (val == EAST || (x < width - 1 && grid->at(y)[x + DX[EAST]] == WEST)) ? std::cout << ' ' : std::cout << '|';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

//------------------------------------------------------------------------------
// The algorithm itself
//------------------------------------------------------------------------------

bool _carvingHelper(int startX, int startY, int newX, int newY, int direction, gridType *grid);
bool _carvePassagesFrom(int startX, int startY, gridType *grid)
{
    // Connects two cells in the grid, subject to constraints.
    // Intended to be called indirectly.

    std::vector<int> directions = {NORTH, SOUTH, EAST, WEST};
    std::random_shuffle(directions.begin(), directions.end());
    for (const auto &direction : directions)
    {
        int newX = startX + DX[direction];
        int newY = startY + DY[direction];

        bool targetInBounds = newY >= 0 && newY < (int)grid->size() &&
                              newX >= 0 && newX < (int)grid->at(0).size();
        if (targetInBounds && grid->at(newY).at(newX) == 0)
        {
            // Queueing tasks lets us more easily control the interval between simulation
            // steps, which makes rendering the state easier
            std::packaged_task<bool()> task(std::bind(_carvingHelper, startX, startY, newX, newY, direction, grid));
            taskDeque.push_front(std::move(task));
        }
    }
    // For our task queue, false indicates that no grid state changes were made
    return false;
}

bool _carvingHelper(int startX, int startY, int newX, int newY, int direction, gridType *grid)
{
    // We need this as its own function, so that one pop of the task queue corresponds to 1 update of the board's state
    // returns true if it changed something, else false
    // TODO: rename this function and make the it & the way it's used less confusing

    // Connect source and target
    // auto DEBUG_1 = grid->at(startY).at(startX);
    // auto DEBUG_2 = grid->at(newY).at(newX);
    bool targetInBounds = newY >= 0 && newY < (int)grid->size() &&
                          newX >= 0 && newX < (int)grid->at(0).size();
    bool cond = targetInBounds && grid->at(newY).at(newX) == 0;

    if (cond)
    {
        mrge.y0 = -1;
        mrge.x0 = -1;
        mrge.x1 = -1;
        mrge.y1 = -1;

        // Don't overwrite existing connections
        if (grid->at(startY).at(startX) == 0)
        {
            grid->at(startY).at(startX) = direction;
            mrge.x0 = startX;
            mrge.y0 = startY;
            // std::cout << "Debug: changed start (" << startY << "," << startX << ") from " << DEBUG_1 << " to " << direction << '\n';
        }
        grid->at(newY).at(newX) = OPPOSITE[direction];
        mrge.x1 = newX;
        mrge.y1 = newY;
        // std::cout << "Debug: changed target (" << newY << "," << newX << ") from " << DEBUG_2 << " to " << OPPOSITE[direction] << std::endl;
    }
    std::packaged_task<bool()> task(std::bind(_carvePassagesFrom, newX, newY, grid));
    taskDeque.push_front(std::move(task));
    return cond;
}

int main()
{
    srand(time(NULL));
    gridType grid = generateMaze();
    displayMaze(&grid);
    return EXIT_SUCCESS;
}
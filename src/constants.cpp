#ifndef CONSTANTS_CPP
#define CONSTANTS_CPP

// Constants used by multiple files should go here
#include <unordered_map>
#include "utils.h"

//------------------------------------------------------------------------------
// Set these constants!
//------------------------------------------------------------------------------

namespace constants {
inline constexpr int ROWS = 15;
inline constexpr int COLS = 15;
inline constexpr int CELLWIDTH = 40;
inline constexpr int CELLHEIGHT = 40;
// How many tasks are allowed to be queued by recursive functions.
// A STACK OVERFLOW can occur at huge values! In my case it occurred at 1320 tasks in queue.
// Exceeding this limit may affect simulation behavior, resulting for example in incomplete mazes being generated
inline constexpr int QUEUE_LENGTH_LIMIT = 500;

// the FPS to use when generating and solving the maze respectively
inline constexpr int FPS_GENERATING = 15;
inline constexpr int FPS_SOLVING = 3;

// Choose one of the available algorithms to generate the maze
enum generatorAlgorithm { RECURSIVE_BACKTRACKING, SILENTLY_GENERATE, ELLERS };
const generatorAlgorithm currentGenerator = RECURSIVE_BACKTRACKING;

// Choose one of the available algorithms to solve the maze
enum solverAlgorithm { NAIVE_RECURSIVE, WEIGHTED_RECURSIVE, SKIP_SOLVING };
const solverAlgorithm currentSolver = NAIVE_RECURSIVE;

// Set the start and end points for the solving algorithm. These values should be 0 indexed
const utils::XY solverStart = {0, 0};
const utils::XY solverEnd = {ROWS - 1, COLS - 1};

// Whether to display the scores of the cells in the maze
const bool displayScores = true;

// The color to use for the endpoint of the maze
const Color mazeEndpointColor = LIGHTGRAY;
// The color to use for the location in the maze currently being examined
const Color cellFocusColor = PURPLE;
// The color to use for the maze walls
const Color wallColor = BLACK;

//------------------------------------------------------------------------------
// Algorithm related constants. Don't edit these.
//------------------------------------------------------------------------------
inline constexpr int NORTH = 1, SOUTH = 2, EAST = 4, WEST = 8;

// The difference in x for each direction
inline std::unordered_map<int, int> DX = {{NORTH, 0}, {SOUTH, 0}, {EAST, 1}, {WEST, -1}};

// The difference in y for each direction
inline std::unordered_map<int, int> DY = {{NORTH, -1}, {SOUTH, 1}, {EAST, 0}, {WEST, 0}};

inline std::unordered_map<int, int> OPPOSITE = {{NORTH, SOUTH}, {SOUTH, NORTH}, {EAST, WEST}, {WEST, EAST}};
}  // namespace constants

#endif /* CONSTANTS_CPP */

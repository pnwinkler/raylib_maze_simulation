#ifndef CONSTANTS_H
#define CONSTANTS_H

// Constants used by multiple files should go here
#include <unordered_map>

//------------------------------------------------------------------------------
// Display related constants
//------------------------------------------------------------------------------

/*
    **WARNING**: don't set the ROWS and COLS values very high, and the CELLWIDTH and CELLHEIGHT values very low. Doing
   so can result in STACK OVERFLOW when executing a recursive algorithm. Your OS should just kill the program, but it's
   possible that it will CRASH instead. In my case it crashed when there were ~1320 tasks in queue.
*/
constexpr int ROWS = 15;
constexpr int COLS = 15;
constexpr int CELLWIDTH = 40;
constexpr int CELLHEIGHT = 40;

// the FPS to use when generating and solving the maze respectively
constexpr int FPS_GENERATING = 15;
constexpr int FPS_SOLVING = 3;

//------------------------------------------------------------------------------
// Algorithm related constants
//------------------------------------------------------------------------------
constexpr int NORTH = 1, SOUTH = 2, EAST = 4, WEST = 8;

// The difference in x for each direction
std::unordered_map<int, int> DX = {{NORTH, 0}, {SOUTH, 0}, {EAST, 1}, {WEST, -1}};

// The difference in y for each direction
std::unordered_map<int, int> DY = {{NORTH, -1}, {SOUTH, 1}, {EAST, 0}, {WEST, 0}};

std::unordered_map<int, int> OPPOSITE = {{NORTH, SOUTH}, {SOUTH, NORTH}, {EAST, WEST}, {WEST, EAST}};

#endif /* CONSTANTS_H */

#include "ellers.h"
#include <algorithm>
#include <cassert>
#include <deque>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include "../../lib/raylib.h"
#include "../constants.cpp"
#include "../utils.h"
#include "unordered_map"
#include "unordered_set"
#include "vector"

// Convention: -1 should be reserved to indicate an uninitialized value.

// Definition: an "absolute index" indicates the location of a cell in a
// 2 dimensional grid, such that for example the cell at [0][1] maps to
// an absolute index of 0*1+1=1, and [5][3] maps to 5*3+3=18

// Map an absolute index (i.e. cell position on a grid) to a group.
static int g_mapIdxToGroup[constants::ROWS * constants::COLS] = {-1};

// Map absolute indices to the direction(s) that each cell tries to connect
static int g_idxToDirection[constants::ROWS * constants::COLS] = {-1};

// Map group numbers (as keys) to a vector of absolute indices
static std::unordered_map<int, std::unordered_set<int>> g_mapGroupToIdx;

// 0 indexed. This is the row we're currently updating
static int g_currentRow = 0;
static int g_maxGroupNrSeen = 0;

// Holds tasks required for simulation, in a queue, to be called later
// std::deque<std::packaged_task<bool()>> el::taskDeque;

using namespace constants;

void el::simulationTick() {
    // todo: update this to actually do ticks, not entire row updates

    if (g_mapIdxToGroup[0] == -1) {
        // perform first-time setup
        std::fill_n(g_mapIdxToGroup, constants::ROWS * constants::COLS, -1);
        std::fill_n(g_idxToDirection, constants::ROWS * constants::COLS, -1);

        srand(time(NULL));
        for (int i = 0; i < constants::ROWS; i++) {
            el::insertGroupMapping(i, i);
            g_maxGroupNrSeen = i;
        }
    }

    /*
        Randomly join adjacent cells that aren't in the same set.
        Any two members of a set have a path connecting them.
    */

    // Absolute index of the item in column 0 of the current and next row respectively
    const int rowBaseIdx = g_currentRow * constants::COLS;
    const int nextRowBaseIdx = (g_currentRow + 1) * constants::COLS;
    if (g_currentRow == constants::ROWS - 1) {
        // last row. Connect all cells in row
        for (int c = 0; c < constants::COLS; c++) {
            conditionallyMergeGroups(rowBaseIdx, rowBaseIdx + c);
        }
        g_currentRow += 1;
        // DEBUG code: print the contents of vecIdxToGroup
        // for (int i = 0; i < constants::ROWS; i++) {
        //     for (int j = 0; j < constants::COLS; j++) {
        //         std::cout << vecIdxToGroup.at(i * constants::COLS + j) << ' ';
        //     }
        //     std::cout << '\n';
        // }
        // auto grid = el::exportCardinalMaze();
        // InitWindow(constants::COLS * constants::CELLWIDTH, constants::ROWS * constants::CELLHEIGHT,
        //            "Eller's Algorithm");
        // el::_nonWasmFuncToDisplayMazeBuildSteps(&grid);
        return;
    }

    for (int c = 0; c < constants::COLS - 1; c++) {
        int shouldMerge = rand() % 2;
        if (shouldMerge != 0) {
            conditionallyMergeGroups(rowBaseIdx + c, rowBaseIdx + c + 1);
        }
        // DEBUG code
        // auto grid = el::exportCardinalMaze();
        // InitWindow(constants::COLS * constants::CELLWIDTH, constants::ROWS * constants::CELLHEIGHT,
        //            "Eller's Algorithm");
        // el::_nonWasmFuncToDisplayMazeBuildSteps(&grid);
    }

    /*
        For each group in the current row, pick at least one of its members
        to connect downwards to the next row
    */

    // Setup 1: pair groups in active row with their indices
    std::unordered_map<int, std::vector<int>> activeRowGroups = {};
    for (int c = 0; c < constants::COLS; c++) {
        int groupNr = g_mapIdxToGroup[rowBaseIdx + c];
        activeRowGroups[groupNr].push_back(rowBaseIdx + c);
    }

    // Setup 2: select a random index from each group to connect downwards
    std::unordered_set<int> connectingDownwards = {};
    for (const auto& [_groupNr, cellIndices] : activeRowGroups) {
        int sourceIdx = cellIndices.at(rand() % cellIndices.size());
        connectingDownwards.insert(sourceIdx);
        // decide whether to connect the other cells in the group
        for (const auto idx : cellIndices) {
            if (idx == sourceIdx) {  // || cellIndices.size() == 1) {
                continue;
            }
            if (rand() % 10 == 1) {
                // too great a chance of connecting downwards results in boring maze
                connectingDownwards.insert(cellIndices.at(idx));
            }
        }
    }

    // Execution: create the connections.
    for (int i = 0; i < constants::COLS; i++) {
        int sourceIdx = rowBaseIdx + i;
        int targetIdx = nextRowBaseIdx + i;

        if (connectingDownwards.contains(sourceIdx)) {
            const int groupNr = g_mapIdxToGroup[sourceIdx];
            el::insertGroupMapping(groupNr, targetIdx);
            g_idxToDirection[sourceIdx] += SOUTH;
            g_idxToDirection[targetIdx] += NORTH;
        } else {
            // set to new groupNr
            g_maxGroupNrSeen += 1;
            el::insertGroupMapping(g_maxGroupNrSeen, targetIdx);
        }
    }

    // DEBUG code
    // auto grid = el::exportCardinalMaze();
    // InitWindow(constants::COLS * constants::CELLWIDTH, constants::ROWS * constants::CELLHEIGHT, "Eller's Algorithm");
    // el::_nonWasmFuncToDisplayMazeBuildSteps(&grid);

    g_currentRow += 1;
}

void el::_nonWasmFuncToDisplayMazeBuildSteps(const gridType& grid) {
    SetTargetFPS(constants::FPS_GENERATING);
    while (!WindowShouldClose())  // Detect window close button or ESC key
    {
        BeginDrawing();
        // el::simulationTick();
        el::_simulationDraw(grid);
        EndDrawing();
    }
    CloseWindow();
}

void el::insertGroupMapping(const int groupNr, const int idx) {
    // Update our structures that track group membership, using the passed-in values

    // Insert a new group mapping, or update the existing mappings
    if (!g_mapGroupToIdx.contains(groupNr)) {
        g_mapGroupToIdx.insert({groupNr, {idx}});
    } else {
        g_mapGroupToIdx.at(groupNr).insert(idx);
    }

    // assert(vecIdxToGroup[idx] != -1);
    g_mapIdxToGroup[idx] = groupNr;

    // 0 indicates that we don't connect in any direction. That's our default behavior.
    // assert(vecIdxToDirection.size() <= idx);
    g_idxToDirection[idx] = 0;
}

void el::conditionallyMergeGroups(const int idxLeft, const int idxRight) {
    // Merge the group at idxRight's location into the group at idxLeft's location, if they're of different groups, then
    // update mappings. The two indices must be direct left and right neighbors.
    assert(idxLeft + 1 == idxRight);

    const auto leftGroupNr = g_mapIdxToGroup[idxLeft];
    const auto rightGroupNr = g_mapIdxToGroup[idxRight];
    if (leftGroupNr == rightGroupNr) {
        return;
    }

    auto& indicesGroupLeft = g_mapGroupToIdx.at(leftGroupNr);
    const auto& indicesGroupRight = g_mapGroupToIdx.at(rightGroupNr);
    for (const int idx : indicesGroupRight) {
        indicesGroupLeft.insert(idx);
        g_mapIdxToGroup[idx] = leftGroupNr;
    }
    g_mapGroupToIdx.erase(rightGroupNr);

    // Update the direction of the cells
    // assert(vecIdxToDirection.size() > idxLeft);
    if ((g_idxToDirection[idxLeft] & EAST) == 0) {
        g_idxToDirection[idxLeft] += EAST;
    }
    if ((g_idxToDirection[idxRight] & WEST) == 0) {
        g_idxToDirection[idxRight] += WEST;
    }
}

void el::generateMazeInstantlyNoDisplay() {
    while (g_currentRow < constants::ROWS) {
        el::simulationTick();
    }
}

void el::_simulationDraw(const gridType& grid) { 
    // Helps draw grid state in GUI. Expects an existing window.
    ClearBackground(RAYWHITE);
    for (int y = 0; y < grid.size(); y++) {
        for (int x = 0; x < grid.at(0).size(); x++) {
            int val = grid.at(y).at(x);

            // DEBUG code
            if ((y * constants::COLS + x) > std::size(g_mapIdxToGroup) - 1) {
                continue;
            }

            std::cout << x << ',' << y << '\n';
            // Draw the group number
            // TODO: stop this from out of range core dumping
            // std::cout<<x<<','<<y<<'\n';
            // TODO: resolve why 0 size
            std::cout << std::size(g_mapIdxToGroup) << '\n';
            int idx = y * constants::COLS + x;
            // if (vecIdxToGroup.size() < idx + 1) {
            //     continue;
            // }

            DrawText(std::to_string(g_mapIdxToGroup[idx]).c_str(), x * CELLWIDTH + 5, y * CELLHEIGHT + 5, 6, BLACK);

            bool northBlocked =
                utils::inBounds(grid, x, y - 1) && (grid.at(y - 1).at(x) & SOUTH) == 0 && (val & NORTH) == 0;
            bool southBlocked =
                utils::inBounds(grid, x, y + 1) && (val & SOUTH) == 0 && (grid.at(y + 1).at(x) & NORTH) == 0;
            bool eastBlocked =
                utils::inBounds(grid, x + 1, y) && (val & EAST) == 0 && (grid.at(y).at(x + 1) & WEST) == 0;
            bool westBlocked =
                utils::inBounds(grid, x - 1, y) && (grid.at(y).at(x - 1) & EAST) == 0 && (val & WEST) == 0;

            if (northBlocked) {
                DrawLine(x * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, y * CELLHEIGHT, BLACK);
            }

            if (southBlocked) {
                DrawLine(x * CELLWIDTH, (y + 1) * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            }

            if (eastBlocked) {
                DrawLine((x + 1) * CELLWIDTH, y * CELLHEIGHT, (x + 1) * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            }

            if (westBlocked) {
                DrawLine(x * CELLWIDTH, y * CELLHEIGHT, x * CELLWIDTH, (y + 1) * CELLHEIGHT, BLACK);
            }
        }
    }
}

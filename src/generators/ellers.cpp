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
static int vecIdxToGroup[constants::ROWS * constants::COLS] = {-1};

// Map absolute indices to the direction(s) that each cell tries to connect
static int vecIdxToDirection[constants::ROWS * constants::COLS] = {-1};

// Map group numbers (as keys) to a vector of absolute indices
static std::unordered_map<int, std::unordered_set<int>> mapGroupToIdx;

// 0 indexed. This is the row we're currently updating
static int currentRow = 0;
static int maxGroupNrSeen = 0;

// Holds tasks required for simulation, in a queue, to be called later
// std::deque<std::packaged_task<bool()>> el::taskDeque;

using namespace constants;

void el::simulationTick() {
    // todo: update this to actually do ticks, not entire row updates

    if (vecIdxToGroup[0] == -1) {
        // perform first-time setup
        std::fill_n(vecIdxToGroup, constants::ROWS * constants::COLS, -1);
        std::fill_n(vecIdxToDirection, constants::ROWS * constants::COLS, -1);

        srand(time(NULL));
        for (int i = 0; i < constants::ROWS; i++) {
            el::insertGroupMapping(i, i);
            maxGroupNrSeen = i;
        }
    }

    /*
        Randomly join adjacent cells that aren't in the same set.
        Any two members of a set have a path connecting them.
    */

    // Absolute index of the item in column 0 of the current and next row respectively
    const int rowBaseIdx = currentRow * constants::COLS;
    const int nextRowBaseIdx = (currentRow + 1) * constants::COLS;
    if (currentRow == constants::ROWS - 1) {
        // last row. Connect all cells in row
        for (int c = 0; c < constants::COLS; c++) {
            conditionallyMergeGroups(rowBaseIdx, rowBaseIdx + c);
        }
        currentRow += 1;
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

    // Setup 1: pair groups in active row with their indices.
    std::unordered_map<int, std::vector<int>> activeRowGroups;
    for (int c = 0; c < constants::COLS; c++) {
        int groupNr = vecIdxToGroup[rowBaseIdx + c];
        activeRowGroups[groupNr].push_back(rowBaseIdx + c);
    }

    // Setup 2: select a random index from each group to connect downwards
    std::unordered_set<int> connectingDownwards;
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
            int groupNr = vecIdxToGroup[sourceIdx];
            el::insertGroupMapping(groupNr, targetIdx);
            vecIdxToDirection[sourceIdx] += SOUTH;
            vecIdxToDirection[targetIdx] += NORTH;
        } else {
            // set to new groupNr
            maxGroupNrSeen += 1;
            el::insertGroupMapping(maxGroupNrSeen, targetIdx);
        }
    }

    // DEBUG code
    // auto grid = el::exportCardinalMaze();
    // InitWindow(constants::COLS * constants::CELLWIDTH, constants::ROWS * constants::CELLHEIGHT, "Eller's Algorithm");
    // el::_nonWasmFuncToDisplayMazeBuildSteps(&grid);

    currentRow += 1;
}

void el::_nonWasmFuncToDisplayMazeBuildSteps(void* arg) {
    gridType* grid_ptr = static_cast<gridType*>(arg);
    SetTargetFPS(constants::FPS_GENERATING);
    while (!WindowShouldClose())  // Detect window close button or ESC key
    {
        BeginDrawing();
        // el::simulationTick();
        el::_simulationDraw(grid_ptr);
        EndDrawing();
    }
    CloseWindow();
}

void el::insertGroupMapping(int groupNr, int idx) {
    // Update our structures that track group membership, using the passed-in values

    // Insert a new group mapping, or update the existing mappings
    if (!mapGroupToIdx.contains(groupNr)) {
        mapGroupToIdx.insert({groupNr, {idx}});
    } else {
        mapGroupToIdx.at(groupNr).insert(idx);
    }

    // assert(vecIdxToGroup[idx] != -1);
    vecIdxToGroup[idx] = groupNr;

    // By default, don't connect in any direction
    // assert(vecIdxToDirection.size() <= idx);
    vecIdxToDirection[idx] = 0;
}

void el::conditionallyMergeGroups(int idxLeft, int idxRight) {
    // Merge the group at idxRight's location into the group at idxLeft's location, if they're of different groups, then
    // update mappings. The two indices must be direct left and right neighbors.
    assert(idxLeft + 1 == idxRight);

    auto leftGroupNr = vecIdxToGroup[idxLeft];
    auto rightGroupNr = vecIdxToGroup[idxRight];
    if (leftGroupNr == rightGroupNr) {
        return;
    }

    auto& indicesGroup1 = mapGroupToIdx.at(leftGroupNr);
    const auto& indicesGroup2 = mapGroupToIdx.at(rightGroupNr);
    for (int idx : indicesGroup2) {
        indicesGroup1.insert(idx);
        vecIdxToGroup[idx] = leftGroupNr;
    }
    mapGroupToIdx.erase(rightGroupNr);

    // Update the direction of the cells
    // assert(vecIdxToDirection.size() > idxLeft);
    if ((vecIdxToDirection[idxLeft] & EAST) == 0) {
        vecIdxToDirection[idxLeft] += EAST;
    }
    if ((vecIdxToDirection[idxRight] & WEST) == 0) {
        vecIdxToDirection[idxRight] += WEST;
    }
}

void el::generateMazeInstantlyNoDisplay() {
    while (currentRow < constants::ROWS) {
        el::simulationTick();
    }
}

gridType el::exportCardinalMaze() {
    // Create and return a maze that has connections between cells saved in terms of cardinal directions (e.g. North,
    // East). Such a format is expected by some of our solvers.
    gridType grid = utils::createEmptyGrid(constants::ROWS, constants::COLS);

    // DEBUG code
    // std::cout << "\n\n";
    // for (int i = 0; i < constants::ROWS; i++) {
    //     for (int j = 0; j < constants::COLS; j++) {
    //         std::cout << vecIdxToGroup.at(i * constants::COLS + j) << ' ';
    //     }
    //     std::cout << '\n';
    // }

    int idx = 0;
    for (int idx = 0; idx < std::size(vecIdxToDirection); idx++) {
        int r = idx / constants::COLS;
        int c = idx % constants::COLS;
        grid.at(r).at(c) = vecIdxToDirection[idx];
    }
    return grid;
}

void el::_simulationDraw(gridType* grid) {
    // Helps draw grid state in GUI. Expects an existing window.
    ClearBackground(RAYWHITE);
    for (int y = 0; y < grid->size(); y++) {
        for (int x = 0; x < grid->at(0).size(); x++) {
            int val = grid->at(y).at(x);

            // DEBUG code
            if ((y * constants::COLS + x) > std::size(vecIdxToGroup) - 1)
                continue;

            std::cout << x << ',' << y << '\n';
            // Draw the group number
            // TODO: stop this from out of range core dumping
            // std::cout<<x<<','<<y<<'\n';
            // TODO: resolve why 0 size
            std::cout << std::size(vecIdxToGroup) << '\n';
            int idx = y * constants::COLS + x;
            // if (vecIdxToGroup.size() < idx + 1) {
            //     continue;
            // }

            DrawText(std::to_string(vecIdxToGroup[idx]).c_str(), x * CELLWIDTH + 5, y * CELLHEIGHT + 5, 6, BLACK);

            bool northBlocked =
                utils::inBounds(*grid, x, y - 1) && (grid->at(y - 1).at(x) & SOUTH) == 0 && (val & NORTH) == 0;
            bool southBlocked =
                utils::inBounds(*grid, x, y + 1) && (val & SOUTH) == 0 && (grid->at(y + 1).at(x) & NORTH) == 0;
            bool eastBlocked =
                utils::inBounds(*grid, x + 1, y) && (val & EAST) == 0 && (grid->at(y).at(x + 1) & WEST) == 0;
            bool westBlocked =
                utils::inBounds(*grid, x - 1, y) && (grid->at(y).at(x - 1) & EAST) == 0 && (val & WEST) == 0;

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

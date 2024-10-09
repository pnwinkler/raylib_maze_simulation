#ifndef ELLERS_H
#define ELLERS_H

#include "../utils.h"
using namespace utils;

namespace el {

// Maze generating functions
void conditionallyMergeGroups(int idxLeft, int idxRight);
void insertGroupMapping(int groupNr, int idx);
void simulationTick();
void generateMazeInstantlyNoDisplay();

// Export functions
gridType exportCardinalMaze();

// Display functions
void _nonWasmFuncToDisplayMazeBuildSteps(const gridType& grid);
// void _wasmFuncToDisplayMazeBuildSteps(void* arg);
void _simulationDraw(const gridType& grid);

}  // namespace el

#endif /* ELLERS_H */
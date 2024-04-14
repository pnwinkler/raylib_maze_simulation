#ifndef RECURSIVE_BACKTRACKING_H
#define RECURSIVE_BACKTRACKING_H

#include "../utils.h"
using namespace utils;

namespace rb {

void generateMazeInstantlyNoDisplay(gridType* grid);
void simulationTick(gridType* grid);

void _wasmFuncToDisplayMazeBuildSteps(void* arg);
void _nonWasmFuncToDisplayMazeBuildSteps(void* arg);
bool _carvingHelper(const XY& start, const XY& target, const int direction, gridType* grid);
void _simulationDraw(gridType* grid);
bool _carvePassagesFrom(const utils::XY& start, gridType* grid);
}  // namespace rb

#endif /* RECURSIVE_BACKTRACKING_H */

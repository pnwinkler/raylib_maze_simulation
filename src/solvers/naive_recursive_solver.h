#ifndef NAIVE_SOLVER_H
#define NAIVE_SOLVER_H

#include <deque>
#include "../../lib/raylib.h"
#include "../utils.h"

using namespace utils;

namespace ns {
// std::deque<utils::XY> locationsInOrderVisited = {};
// std::unordered_set<int> indicesChecked = {};
Color gradateColor(Color start, Color target, int i, int locationIdx);
bool nextStep(gridType& grid, XY target, std::deque<XY>& locationsToCheck);
void SolverUpdateDrawFrame(void);
void _solverDraw(gridType& grid, int locationIdx);

void animateSolution(gridType& grid);
void naiveSolver(gridType& grid, XY startLoc, XY endLoc);
}  // namespace ns

#endif /* NAIVE_SOLVER_H */
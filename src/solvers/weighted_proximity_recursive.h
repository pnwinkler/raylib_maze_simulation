#ifndef WEIGHTED_SOLVER_H
#define WEIGHTED_SOLVER_H

#include <deque>
#include "../../lib/raylib.h"
#include "../utils.h"
#include "unordered_set"

using namespace utils;

namespace ws {

bool nextStep(gridType& grid, XY target);
int calculateWeight(XY cell, XY mazeFinish);
void animateSolution(gridType& grid);
void insertIntoScores(XY cell, int score);
void solve(gridType& grid, XY startLoc, XY endLoc);

void _solverDraw(gridType& grid, int locationIdx);
}  // namespace ws

#endif /* WEIGHTED_SOLVER_H */
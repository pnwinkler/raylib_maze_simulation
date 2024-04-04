#ifndef NAIVE_SOLVER_H
#define NAIVE_SOLVER_H

#include <deque>
#include "../../lib/raylib.h"
#include "../utils.h"

using namespace utils;

namespace ns {

Color gradateColor(Color start, Color target, int i, int locationIdx);
bool nextStep(gridType& grid, XY target, std::deque<XY>& locationsToCheck);
void animateSolution(gridType& grid);
void solve(gridType& grid, XY startLoc, XY endLoc);

void _solverDraw(gridType& grid, int locationIdx);

}  // namespace ns

#endif /* NAIVE_SOLVER_H */
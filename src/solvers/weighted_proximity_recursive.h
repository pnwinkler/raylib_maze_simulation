#ifndef WEIGHTED_SOLVER_H
#define WEIGHTED_SOLVER_H

#include <deque>
#include "../../lib/raylib.h"
#include "../utils.h"
#include "map"
#include "unordered_set"

using namespace utils;

namespace ws {

// Scores (as keys) for cells (as values) for all cells that have not yet been visited.
typedef std::map<int, std::unordered_set<XY>> tScores;

bool nextStep(const gridType& grid, const XY& target, tScores& remainingScores);
int calculateScore(const XY& cell, const XY& mazeFinish);
XY popBestScorer(tScores& remainingScores);
void animateSolution(const gridType& grid);
void insertIntoScores(const XY& cell, const int score, tScores& remainingScores);
void solve(const gridType& grid, const XY& startLoc, const XY& endLoc);

void _solverDraw(const gridType& grid, const int locationIdx);
}  // namespace ws

#endif /* WEIGHTED_SOLVER_H */

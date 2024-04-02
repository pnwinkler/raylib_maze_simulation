#ifndef RECURSIVE_BACKTRACKING_H
#define RECURSIVE_BACKTRACKING_H

#include "../utils.h"
using namespace utils;

namespace RB {
class recursive_backtracking {
   public:
    void displayMazeBuildSteps(gridType* grid);
    void generateMazeInstantlyNoDisplay(gridType* grid);
    void simulationTick(gridType* grid);

//    private:
    void _WasmMazeGeneratorDraw(void* arg);
    bool _carvingHelper(const XY& start, const XY& target, const int direction, gridType* grid);
    void _simulationDraw(gridType* grid);
    bool _carvePassagesFrom(const utils::XY& start, gridType* grid);
};
}  // namespace RB
#endif /* RECURSIVE_BACKTRACKING_H */

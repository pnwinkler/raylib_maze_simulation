#ifndef RECURSIVE_BACTRACKING_H
#define RECURSIVE_BACTRACKING_H

#include "../utils.cpp"
using namespace constants;

namespace RB {
class recursive_backtracking {
   public:
    void WasmMazeGeneratorUpdateDrawFrame(void* arg);
    void displayMazeBuildSteps(gridType* grid);
    void generateMazeInstantlyNoDisplay(gridType* grid);
    void simulationTick(gridType* grid);

   private:
    bool _carvingHelper(const utils::XY& start, const utils::XY& target, const int direction, gridType* grid);
    void _simulationDraw(gridType* grid);
    bool _carvePassagesFrom(const utils::XY& start, gridType* grid);
};
}  // namespace RB
#endif /* RECURSIVE_BACTRACKING_H */

#ifndef RECURSIVE_BACTRACKING_H
#define RECURSIVE_BACTRACKING_H

#include "../utils.cpp"

namespace RB {
class recursive_backtracking {
   public:
    void WasmMazeGeneratorUpdateDrawFrame(void* arg);
    void displayMazeBuildSteps(gridType* grid);
    void generateMazeInstantly(gridType* grid);
    bool carvingHelper(const XY& start, const XY& target, const int direction, gridType* grid);
    void simulationTick(gridType* grid);
   private:
    void _simulationDraw(gridType* grid);
    bool carvePassagesFrom(const XY& start, gridType* grid);
};
}  // namespace RB
#endif /* RECURSIVE_BACTRACKING_H */

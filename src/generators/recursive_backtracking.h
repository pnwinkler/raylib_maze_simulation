#ifndef RECURSIVE_BACTRACKING_H
#define RECURSIVE_BACTRACKING_H

#include "utils.cpp"

namespace RB
{
  // bool _carvePassagesFrom(int startX, int startY, gridType* grid) ;
  bool carvingHelper(int startX, int startY, int newX, int newY, int direction, gridType* grid) ;
  void displayMazeInConsole(gridType* grid) ;
  // void _simulationDraw(gridType* grid) ;
  void displayMazeIterations(gridType* grid);
  void generateMazeInstantly(gridType* grid) ;
  void simulationTick(gridType *grid);
}

#endif /* RECURSIVE_BACTRACKING_H */

// This is the abstract base class for maze generating algorithms to adhere to.

#include "../constants.cpp"
#include "../utils.h"

class MazeGeneratorABC {
   public:
    // Constructor
    MazeGeneratorABC() {
        static_assert(constants::FPS_GENERATING > 0, "FPS must be greater than 0");
        static_assert(constants::FPS_SOLVING > 0, "FPS must be greater than 0");
    }

    // Generate a maze instantly using the algorithm, without animation or saving of intermediate results.
    virtual utils::gridType* generateMazeInstantly() = 0;

    // Generate a maze one step at a time. This function should return True once the maze is fully generated, else
    // False. It may be advisable for the implementer to "fast-forward" over steps that don't change the maze state.
    virtual bool generateMazeStepwise() = 0;

    // Return True once the maze is fully generated, else False.
    virtual bool mazeGenerationComplete() const = 0;

    // Draw the maze state graphically, without handling window creation, destruction, beginDrawing(), endDrawing() etc.
    // The implementer should select and return the correct WASM or non-WASM implementation.
    virtual void drawMazeFunction(utils::gridType* grid) = 0;
};
// Implement all functionality for the display of the maze simulation here
// The main things this does are:
//  1) centralize much of the display logic
//  2) allow us to transition "scenes" (by which I mean, the display of the maze generation and solving respectively)

#include "../../lib/raylib.h"
#include "../constants.cpp"
#include "../utils.h"

class SimGui {
   public:
    int canvasWidth;
    int canvasHeight;
    bool canTerminateDrawFunc = false;

    SimGui() {
        this->_simGUIsetCanvasDimensions();
        InitWindow(this->canvasWidth, this->canvasHeight, "Maze Solver");
        _drawFuncs = {};
    }

    ~SimGui() { CloseWindow(); }

    // void setTitle(const char* title) { SetWindowTitle(title); }
    void queueDrawFunc(bool (*drawFunc)()) { _drawFuncs.push_back(drawFunc); }

   private:
    // create a list of functions to be executed in the draw loop
    // todo: consider having this be a vector of structs instead, where each struct contains the function, perhaps a
    // title, and other configurables etc
    std::vector<bool (*)()> _drawFuncs;

    // Once the draw function has completed, we can execute the next draw function in the queue
    // We expect each draw function to return a boolean value indicating whether it is ready to be terminated
    void executeDrawFunc() {
        auto drawFunc = _drawFuncs.front();
        canTerminateDrawFunc = drawFunc();
        if (canTerminateDrawFunc) {
            if (_drawFuncs.size() > 0) {
                _drawFuncs.erase(_drawFuncs.begin());
            }
        }
    }

    void _simGUIsetCanvasDimensions() {
        this->canvasWidth = constants::COLS * constants::CELLWIDTH;
        this->canvasHeight = constants::ROWS * constants::CELLHEIGHT;
    }
};
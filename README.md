# About
This code generates a maze graphically (displayed using Raylib), and solves it.

# Maze-generating algorithms implemented:
- Recursive backtracking
- Ellers (algorithm complete, display functionality is WIP)

# Maze-solving algorithms implemented:
- Naive recursive algorithm
- Proximity-weighted recursive algorithm

# Caveats
This is my first project using: 
- C++
- Computer graphics
- Maze-related algorithms

Therefore, this project's code style and logical structure should not necessarily be considered good practice.

# How to build for offline use
- Clone
- cd into folder
- configure params in constants.cpp
- run this command: `make PLATFORM=PLATFORM_DESKTOP`
- run the executable from bin, e.g. `./bin/main`

# How to build for web using WASM

WASM is currently not supported. Supporting it was causing too many headaches. I may support it again in future. 

## Useful resources 
Some of these docs are slightly out of date, but they should convey the gist of how WASM, Emscripten, and Raylib operate together.

Mozilla's WASM docs https://developer.mozilla.org/en-US/docs/WebAssembly/C_to_wasm  
Emscripten docs https://emscripten.org/docs/compiling/Building-Projects.html#building-projects  
Raylib docs https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)

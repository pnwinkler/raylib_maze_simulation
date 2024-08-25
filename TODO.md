# Todo's:

## Short / Mid-term:
- improve the graphical display by:
- consider adding stats, like % cells visited, steps performed, dead ends encountered, etc
- make a proximity based recursive solver once done with this solver
- see if we can "statically link" (or whatever it's called) the relevant parts of raylib when compiling

## Long-term:
- Limit the number of tasks that can be queued, to prevent issues
- Run a memory safety / memory leak checker
- Write in Go or Rust
- Update README to explain how to use and install
- Dockerize?
- <Do other task once the core C++ project is done>
- consider showing the comparison or command being executed on screen (i.e. to demonstrate the reason why something was chosen)
- ...


## NOTES to self 

Old WASM instructions (can copy paste back later).
- Clone
- cd into folder
- configure params in constants.cpp
- run this command: make PLATFORM=PLATFORM_WEB
- run this command: python -m http.server 8000
- open in your browser the html file, like this http://localhost:8000/path/to/game.html. Provide the full path to the html file, minus the home/username bit, and make sure the port matches that used by the python command above.


source ~/git/emsdk/emsdk_env.fish

ls src/**.cpp | entr -p sh -c "make clean && clear && printf '\e[3J' && make PLATFORM=PLATFORM_DESKTOP BUILD_MODE=DEBUG && clear && printf '\e[3J' && ./bin/main"
ls src/**.cpp | entr -p sh -c "make clean && clear && printf '\e[3J' && make PLATFORM=PLATFORM_WEB BUILD_MODE=DEBUG && clear && printf '\e[3J' && firefox --private-window http://localhost:8000/git/personal/C_CPP_lang/raylib/0/game.html"

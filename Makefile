# Define required environment variables
#------------------------------------------------------------------------------------------------
# Define target platform: PLATFORM_DESKTOP, PLATFORM_WEB
PLATFORM             ?= PLATFORM_DESKTOP

# Define source code path
SRC_PATH      ?= ./src
OBJS = $(SRC_PATH:.cpp=.o)

# Build mode for library: DEBUG or RELEASE
BUILD_MODE    ?= RELEASE

# Define default C compiler to pack library: CC
#------------------------------------------------------------------------------------------------
# TODO: see if we can get rid of the path, make it more platform agnostic
CC =/usr/bin/g++


# Define compiler flags: CFLAGS
#------------------------------------------------------------------------------------------------
#  -std=c++23               defines C++ language mode (standard C++ from 2023 revision)
CFLAGS= -std=c++23 
WASM_OUT= game.html
SHELL_HTML= shell.html

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
		#  These flags are copy-pasted from the Raylib documentation
		CFLAGS += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

ifeq ($(PLATFORM),PLATFORM_WEB)
    # HTML5 emscripten compiler
    CC = emcc
		#  -Os                       	  optimize for code size
		#  -Wall                        enable most warnings
		#  --shell-file 								specify the shell file, that will serve as a launcher to run the code
		#  -DPLATFORM_WEB 							compile for use in the web browser
		#  The other flags are required for Raylib or code in $(SRC_PATH)
		CFLAGS += -o $(WASM_OUT) -Os -Wall ./lib/libraylib.a -I. -I./lib/libraylib.h -L. -L./lib/libraylib.a -s USE_GLFW=3 --shell-file $(SHELL_HTML) -DPLATFORM_WEB
		ifeq ($(BUILD_MODE),DEBUG)
				# -sASSERTIONS=1            enable runtime checks for common memory allocation errors (-O1 and above turn it off)
				# --profiling               include information for code profiling
				CFLAGS += -sASSERTIONS=1 --profiling
		endif
endif

ifeq ($(BUILD_MODE),DEBUG)
		#  -g                       include debug information on compilation
		#  -pedantic-errors         force compilation to fail if attempting to compile code not adhering to C/C++ standards
		CFLAGS += -g -pedantic-errors
endif

# Define source code object files required
#------------------------------------------------------------------------------------------------
# OBJ=bin/recursive_backtracking.o

naive_recursive_solver: $(SRC_PATH)/solvers/naive_recursive_solver.cpp
	$(CC) -o bin/$@ $^ $(CFLAGS) $(SRC_PATH)/generators/recursive_backtracking.cpp 

clean:
	# rm -f $(OBJ)
	rm -f $(basename $(WASM_OUT)).html $(basename $(WASM_OUT)).wasm $(basename $(WASM_OUT)).js

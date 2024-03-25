CC=/usr/bin/g++
CFLAGS=-I=./src -g -pedantic-errors -std=c++23 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
DEPS =
OBJ=bin/recursive_backtracking.o

# Rule to build object files
bin/%.o: src/generators/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bin/%.o: src/solvers/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	

# Rule to build the final executable
solver: src/solvers/naive_solver.cpp
	$(CC) -o bin/$@ $^ $(CFLAGS)

recursive_backtracking: $(OBJ)
	$(CC) -o bin/$@ $^ $(CFLAGS)

clean:
	rm -f $(OBJ)

CC=/usr/bin/g++
CFLAGS=-I=./src -g -pedantic-errors -std=c++23 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
DEPS =
OBJ=bin/recursive_backtracking.o

# Rule to build object files
bin/%.o: src/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Rule to build the final executable
recursive_backtracking: $(OBJ)
	$(CC) -o bin/$@ $^ $(CFLAGS)

clean:
	rm -f $(OBJ)

// Constants used by multiple files should go here
#include <unordered_map>

constexpr int NORTH = 1, SOUTH = 2, EAST = 4, WEST = 8;

// The difference in x for each direction
std::unordered_map<int, int> DX = {
    {NORTH, 0},
    {SOUTH, 0},
    {EAST, 1},
    {WEST, -1}};

// The difference in y for each direction
std::unordered_map<int, int> DY = {
    {NORTH, -1},
    {SOUTH, 1},
    {EAST, 0},
    {WEST, 0}};

std::unordered_map<int, int> OPPOSITE = {
    {NORTH, SOUTH},
    {SOUTH, NORTH},
    {EAST, WEST},
    {WEST, EAST}};

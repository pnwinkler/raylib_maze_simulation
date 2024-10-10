#include <cassert>  // for assert
#include <cstdlib>  // for std::abort
#include <iostream>
#include "../src/constants.cpp"
#include "../src/utils.h"

int testCreateEmptyGrid() {
    auto grid = utils::createEmptyGrid(5, 5);
    assert(grid.size() == 5);
    assert(grid.at(0).size() == 5);
    // arbitrary locations. We just want all locations to have initial value of 0
    assert(grid.at(0).at(0) == 0);
    assert(grid.at(3).at(2) == 0);
    assert(grid.at(4).at(4) == 0);

    return 0;
}

// Test the utils::inBounds function for integer X and Y arguments
int testInBounds() {
    auto grid_1 = utils::createEmptyGrid(1, 1);
    assert(utils::inBounds(grid_1, 0, 0));
    assert(!utils::inBounds(grid_1, 1, 1));

    auto grid_2 = utils::createEmptyGrid(3, 3);
    assert(utils::inBounds(grid_2, 1, 1));
    assert(utils::inBounds(grid_2, 0, 2));
    assert(!utils::inBounds(grid_2, 0, 3));

    return 0;
}

// Test the utils::inBounds function for the XY struct
int testInBoundsXY() {
    auto grid_1 = utils::createEmptyGrid(1, 1);
    assert(utils::inBounds(grid_1, utils::XY{0, 0}));
    assert(!utils::inBounds(grid_1, utils::XY{1, 1}));

    auto grid_2 = utils::createEmptyGrid(3, 3);
    assert(utils::inBounds(grid_2, utils::XY{1, 1}));
    assert(utils::inBounds(grid_2, utils::XY{0, 2}));
    assert(!utils::inBounds(grid_2, utils::XY{0, 3}));

    return 0;
}

// Test the utils::gradateColor function doesn't return colors with negative values
int testGradateColor() {
    auto c1 = Color{50, 50, 50, 50};
    // TODO: consider testing equal color or off by one inputs
    // auto c2 = Color{50, 50, 50, 50};
    auto c2 = Color{75, 75, 75, 75};

    // TODO: make these comments clearer. What does this mean?
    // If the start color is found at the maximum index, then its color should remain unchanged
    Color res_1 = utils::gradateColor(c1, c2, 1, 1);
    assert(res_1.r == c1.r);
    assert(res_1.g == c1.g);
    assert(res_1.b == c1.b);
    assert(res_1.a == c1.a);

    // If the start color is found at the maximum index, then its color should remain unchanged
    Color res_2 = utils::gradateColor(c1, c2, 2, 2);
    assert(res_2.r == c1.r);
    assert(res_2.g == c1.g);
    assert(res_2.b == c1.b);
    assert(res_2.a == c1.a);

    // If the start color is not found at the maximum index, then its color should change to
    // be more like the end color
    Color res_3 = utils::gradateColor(c1, c2, 1, 2);
    assert(res_3.r > c1.r);
    assert(res_3.g > c1.g);
    assert(res_3.b > c1.b);
    assert(res_3.a > c1.a);
    assert(res_3.r < c2.r);
    assert(res_3.g < c2.g);
    assert(res_3.b < c2.b);
    assert(res_3.a < c2.a);

    // Test where the start color has greater values than the target color
    Color res_4 = utils::gradateColor(c2, c1, 1, 2);
    assert(res_4.r > c1.r);
    assert(res_4.g > c1.g);
    assert(res_4.b > c1.b);
    assert(res_4.a > c1.a);
    assert(res_4.r < c2.r);
    assert(res_4.g < c2.g);
    assert(res_4.b < c2.b);
    assert(res_4.a < c2.a);

    // Results 3 and 4 should be identical
    assert(res_3.r == res_4.r);
    assert(res_3.g == res_4.g);
    assert(res_3.b == res_4.b);
    assert(res_3.a == res_4.a);

    return 0;
}

int testReturnAccessibleNeighbors() {
    // When either the origin or target cell point to the other, we then consider
    // the target cell an accessible neighbor
    auto grid_1 = utils::createEmptyGrid(1, 2);
    grid_1.at(0).at(0) = constants::WEST;
    grid_1.at(0).at(1) = constants::WEST;
    auto res_1 = utils::returnAccessibleNeighbors(grid_1, utils::XY{0, 0}, {});

    // There should be 1 accessible neighbor, to the origin's right
    assert(res_1.size() == 1);
    auto obj_1 = res_1.at(0);
    assert(obj_1.x == 1);
    assert(obj_1.y == 0);

    // Repeat the test, but this time, neither cell points to the other, so we expect 0 accessible neighbors
    auto grid_2 = utils::createEmptyGrid(1, 2);
    grid_2.at(0).at(0) = constants::WEST;
    grid_2.at(0).at(1) = constants::EAST;
    auto res_2 = utils::returnAccessibleNeighbors(grid_2, utils::XY{0, 0}, {});
    assert(res_2.size() == 0);

    // We expect neighbors to be accessible both horizontally and vertically. We don't test for order.
    auto grid_3 = utils::createEmptyGrid(3, 3);
    grid_3.at(1).at(1) = constants::WEST + constants::EAST + constants::NORTH + constants::SOUTH;
    auto res_3 = utils::returnAccessibleNeighbors(grid_3, utils::XY{1, 1}, {});

    // todo: make this less obtuse
    // Check that our expected x and y locations all appear in the returned data structure
    assert(res_3.size() == 4);
    // These are the paired x, y locations that we expect to find in the results.
    std::vector<std::tuple<int, int>> expected_neighbors = {
        // x, y
        {1, 0},  // Northern neighbor
        {0, 1},  // Western neighbor
        {2, 1},  // Eastern neighbor
        {1, 2}   // Southern neighbor
    };
    for (auto&& actual : res_3) {
        for (int i = 0; i < expected_neighbors.size(); i++) {
            auto expected_x = std::get<0>(expected_neighbors[i]);
            auto expected_y = std::get<1>(expected_neighbors[i]);
            if ((expected_x == actual.x) && (expected_y == actual.y)) {
                expected_neighbors.erase(expected_neighbors.begin() + i);
                continue;
            }
        }
    }
    assert(expected_neighbors.size() == 0 && "Failed to pop expected elements off stack");

    return 0;
}

int main() {
    testCreateEmptyGrid();
    testInBounds();
    testInBoundsXY();
    testGradateColor();
    testReturnAccessibleNeighbors();

    std::cout << "All tests succeeded\n";
    return 0;
}
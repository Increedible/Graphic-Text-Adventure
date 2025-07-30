#pragma once

#include <array>
#include <string>

enum Direction {
    north,
    east,
    south,
    west,
};

void MSDelay(int ms);
// [l, r)
int randomnum(int l, int r);
// [0,r)
int randomnum(int r);

// an image of `n` rows of characters
template<int n>
using image=std::array<std::string, n>;

std::string toString(char c);
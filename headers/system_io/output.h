#pragma once
#include <iostream>
#include <cmath>
void set_cursor(bool visible);
void clear();
void goBack(int b);
int rgb_to_ansi256(int r, int g, int b);
void printpx(int r, int g, int b);
void newline();

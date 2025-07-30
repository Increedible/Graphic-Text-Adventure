#include "output.h"

void set_cursor(bool visible) {
    if (visible) {
        std::printf("\033[?25h");
    } else {
        std::printf("\033[?25l");
    }
}

void clear() {
    std::printf("\033[2J\033[H");
}

void goBack(int b) {
    std::printf("\033[%dA", b);
}

int rgb_to_ansi256(int r, int g, int b) {
    static const int levels[6] = {  0,  95, 135, 175, 215, 255 };

    int ri = 0, gi = 0, bi = 0;
    int dr = std::abs(r - levels[0]);
    int dg = std::abs(g - levels[0]);
    int db = std::abs(b - levels[0]);
    for (int i = 1; i < 6; ++i) {
        int d = std::abs(r - levels[i]);
        if (d < dr) { dr = d; ri = i; }
        d = std::abs(g - levels[i]);
        if (d < dg) { dg = d; gi = i; }
        d = std::abs(b - levels[i]);
        if (d < db) { db = d; bi = i; }
    }
    int cube_index = 16 + 36*ri + 6*gi + bi;
    int rc = levels[ri], gc = levels[gi], bc = levels[bi];
    int dist_cube = (r-rc)*(r-rc) + (g-gc)*(g-gc) + (b-bc)*(b-bc);

    int gray = (r + g + b) / 3;
    int gi_gray = (gray - 8 + 5) / 10;
    if (gi_gray < 0)  gi_gray = 0;
    if (gi_gray > 23) gi_gray = 23;
    int gray_level = 8 + 10 * gi_gray;
    int gray_index = 232 + gi_gray;
    int dist_gray = (r-gray_level)*(r-gray_level)
                  + (g-gray_level)*(g-gray_level)
                  + (b-gray_level)*(b-gray_level);

    return (dist_gray < dist_cube ? gray_index : cube_index);
}

void printpx(int r, int g, int b) {
    int idx = rgb_to_ansi256(r, g, b);
    std::printf("\033[48;5;%dm \033[0m", idx);
}

void newline() {
    std::printf("\n");
}


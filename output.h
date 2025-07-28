#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cmath>


#define PIXEL_BLACK "\033[40m \033[0m"
#define PIXEL_RED "\033[41m \033[0m"
#define PIXEL_GREEN "\033[42m \033[0m"
#define PIXEL_YELLOW "\033[43m \033[0m"
#define PIXEL_BLUE "\033[44m \033[0m"
#define PIXEL_MAGENTA "\033[45m \033[0m"
#define PIXEL_CYAN "\033[46m \033[0m"
#define PIXEL_WHITE "\033[47m \033[0m"

std::string colored(std::string text, std::string type, std::string color, std::string color2 = "NULL") {
    std::vector<int> values;
    bool istext = false;
    bool isback = false;
    std::stack<std::string> colors;
    if (type == "text") {
        istext = true;
        colors.push(color);
    }
    else if (type == "back") {
        isback = true;
        colors.push(color);
    }
    else if (type == "backtext") {
        istext = true;
        isback = true;
        colors.push(color);
        if (color2 == "NULL") { istext = false; }
        else { colors.push(color2); }
    }
    else if (type == "textback") {
        istext, isback = true;
        if (color2 == "NULL") { return "ERROR : Need second color"; }
        colors.push(color2);
        colors.push(color);
    }
    else
        return "ERROR: Unvalid type";
    if (istext) {
        std::string pickedcolor = colors.top();
        colors.pop();
        if (pickedcolor == "black") { values.push_back(30); }
        else if (pickedcolor == "red") { values.push_back(31); }
        else if (pickedcolor == "green") { values.push_back(32); }
        else if (pickedcolor == "yellow") { values.push_back(33); }
        else if (pickedcolor == "blue") { values.push_back(34); }
        else if (pickedcolor == "magenta") { values.push_back(35); }
        else if (pickedcolor == "cyan") { values.push_back(36); }
        else if (pickedcolor == "white") { values.push_back(37); }
        else { values.push_back(30); }
    }
    if (isback) {
        std::string pickedcolor = colors.top();
        colors.pop();
        if (pickedcolor == "black") { values.push_back(40); }
        else if (pickedcolor == "red") { values.push_back(41); }
        else if (pickedcolor == "green") { values.push_back(42); }
        else if (pickedcolor == "yellow") { values.push_back(43); }
        else if (pickedcolor == "blue") { values.push_back(44); }
        else if (pickedcolor == "magenta") { values.push_back(45); }
        else if (pickedcolor == "cyan") { values.push_back(46); }
        else if (pickedcolor == "white") { values.push_back(47); }
        else { values.push_back(30); }
    }
    values.push_back(1);
    std::string output;
    for (int i = 0; i < (int)values.size(); i++) {
        output += std::to_string(values[i]);
        if (i < (int)values.size() - 1) { output += ';'; }
    }
    return ("\033[" + output + 'm' + text + "\033[0m");
}

void set_cursor(bool visible) {
    if (visible) {
        printf("\033[?25h");
    } else {
        printf("\033[?25l");
    }
}

void clear() {
    printf("\033[2J\033[H");
}

void goBack(int b) {
    printf("\033[%dA", b);
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

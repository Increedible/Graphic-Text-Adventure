#include "output.h"


#define PIXEL_BLACK   "\033[40m \033[0m"
#define PIXEL_RED     "\033[41m \033[0m"
#define PIXEL_GREEN   "\033[42m \033[0m"
#define PIXEL_YELLOW  "\033[43m \033[0m"
#define PIXEL_BLUE    "\033[44m \033[0m"
#define PIXEL_MAGENTA "\033[45m \033[0m"
#define PIXEL_CYAN    "\033[46m \033[0m"
#define PIXEL_WHITE   "\033[47m \033[0m"

#define COLOR_RESET   "\033[0m"

// enum class Color {
//     Black,
//     Red,
//     Green,
//     Yellow,
//     Blue,
//     Magenta,
//     Cyan,
//     White,
//     Default,
//     None
// };

// constexpr const char *const FG_COLOR[] = {
//     "\033[30m",
//     "\033[31m",
//     "\033[32m",
//     "\033[33m",
//     "\033[34m",
//     "\033[35m",
//     "\033[36m",
//     "\033[37m",
//     "\033[39m"
// };

// constexpr const char *const BG_COLOR[] = {
//     "\033[40m",
//     "\033[41m",
//     "\033[42m",
//     "\033[43m",
//     "\033[44m",
//     "\033[45m",
//     "\033[46m",
//     "\033[47m",
//     "\033[49m"
// };

// string where every non-control character is seprated by \0

StyleString::StyleString(){}

// create StyleString with normal (no control character) string
StyleString::StyleString(const std::string &s){
    for (const char&c:s){
        push_back(c);
    }
}

void StyleString::push_back(const char&c) {
    str.push_back(c);
    str.push_back('\0');
}

StyleString StyleString::operator+(const StyleString& b) {
    StyleString ret;
    ret.str = str+b.str;
    return ret;
}

std::string StyleString::to_string(){
    std::string s;
    for(auto c: s)
        if (c != '\0')
            s += c;      
    return s;
}

std::string StyleString::to_plain_string(){
    std::string s;
    char pc;
    for(auto c: s){
        if (c == '\0'){
            s.push_back(pc);
        }
        pc = c;
    }
    return s;
}

StyleString operator+(const std::string &s, const StyleString &ss) {
    StyleString ret(s);
    ret.str += ss.str;
    return ret;
}

StyleString operator+(StyleString ss, const char* s) {
    return ss+std::string(s);
}

// return string with color code
// require non-empty string
StyleString colored(std::string text, Color fg, Color bg) {
    StyleString ret(text);
    std::string ctrl;
    if (fg != Color::None) ctrl += FG_COLOR[(int)fg];
    if (bg != Color::None) ctrl += BG_COLOR[(int)bg];
    ret.str.pop_back();
    char last_char = ret.str.back();
    ret.str.pop_back();
    ret.str = ctrl + ret.str + COLOR_RESET;
    ret.str.push_back(last_char);
    ret.str.push_back('\0');
    return ret;
}

void printStyle(StyleString str){
    for (const char *i = str.str.c_str();*i;i++) {
        i += printf("%s",i);
    }
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


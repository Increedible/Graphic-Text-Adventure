#pragma once

#include <string>

#define PIXEL_BLACK   "\033[40m \033[0m"
#define PIXEL_RED     "\033[41m \033[0m"
#define PIXEL_GREEN   "\033[42m \033[0m"
#define PIXEL_YELLOW  "\033[43m \033[0m"
#define PIXEL_BLUE    "\033[44m \033[0m"
#define PIXEL_MAGENTA "\033[45m \033[0m"
#define PIXEL_CYAN    "\033[46m \033[0m"
#define PIXEL_WHITE   "\033[47m \033[0m"

#define COLOR_RESET   "\033[0m"

enum class Color {
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    Default,
    None
};

constexpr const char *const FG_COLOR[] = {
    "\033[30m",
    "\033[31m",
    "\033[32m",
    "\033[33m",
    "\033[34m",
    "\033[35m",
    "\033[36m",
    "\033[37m",
    "\033[39m"
};

constexpr const char *const BG_COLOR[] = {
    "\033[40m",
    "\033[41m",
    "\033[42m",
    "\033[43m",
    "\033[44m",
    "\033[45m",
    "\033[46m",
    "\033[47m",
    "\033[49m"
};

// string where every non-control character is seprated by \0
struct StyleString{
    std::string str;
    StyleString();
    StyleString(const std::string &s);
    void push_back(const char&c);
    StyleString operator+(const StyleString& b);
    std::string to_string();
};

StyleString operator+(const std::string &s, const StyleString &ss);
StyleString operator+(StyleString ss, const char* s);
StyleString colored(std::string text, Color fg = Color::None, Color bg = Color::None);
void printStyle(StyleString str);
void set_cursor(bool visible);
void clear();
void goBack(int b);
int rgb_to_ansi256(int r, int g, int b);
void printpx(int r, int g, int b);
void newline();

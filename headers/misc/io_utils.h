#pragma once
// #include <cctype>
// #include <vector>
#include "../system_io/input.h"
#include "../system_io/output.h"
#include <string>

#define PIXEL_BLACK   "\033[40m "
#define PIXEL_RED     "\033[41m "
#define PIXEL_GREEN   "\033[42m "
#define PIXEL_YELLOW  "\033[43m "
#define PIXEL_BLUE    "\033[44m "
#define PIXEL_MAGENTA "\033[45m "
#define PIXEL_CYAN    "\033[46m "
#define PIXEL_WHITE   "\033[47m "

#define PIXEL_RESET   "\033[0m"

enum class Color {
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    None,
};

struct BetterColor {
    int value=-1;
    BetterColor(int num);
    BetterColor(Color num);
    std::string toString() const;
};

// string where every non-control character is seprated by \0
struct StyleString{
    std::string str;
    StyleString();
    StyleString(const std::string &s);
    StyleString(const StyleString &s);
    const char* c_str() const;
    const char* begin() const;
    const char* end() const;
    void operator+=(const std::string &s);
    void operator+=(const StyleString &ss);
    void operator=(const StyleString& s);
    void operator=(const std::string& s);
    void push_back(const char&c);
    std::string to_string() const;
    std::string to_raw_string() const;
};

StyleString operator+(const StyleString &a, const StyleString &b);
StyleString operator+(const std::string &s, const StyleString &ss);
StyleString operator+(const StyleString &ss, const std::string &s);
StyleString operator+(const char* s, const StyleString &ss); 
StyleString operator+(const StyleString &ss, const char* s);
StyleString colored(const std::string &text, BetterColor fg = Color::None, BetterColor bg = Color::None);
void printStyle(const StyleString &str);

// wait for enter key to be pressed
void wait_enter(MyIO &io);
void typeOut(MyIO &io, const StyleString &text, int sleepms = 18, int aftersleep = 0);
void typeOut(MyIO &io, const char* text, int sleepms = 18, int aftersleep = 0);
void typeOutLine(MyIO &io, const StyleString &text, int sleepms = 18, int aftersleep = 0);
void typeOutLine(MyIO &io, const char* text, int sleepms = 18, int aftersleep = 0);

struct Option {
    std::string text;
    int key;
    Color color, selected_color;
    Option(const std::string &text_arg,
        int key_arg,
        Color color_arg = Color::White,
        Color selected_color_arg = Color::Yellow);
};

int optionsNav(MyIO &io, const std::vector<Option> &options, std::string hint = "Select");

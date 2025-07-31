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
    BetterColor(int num) {
        value = num;
    }
    BetterColor(Color num) {
        if (num==Color::None) value=-1;
        else value = (int)num+8; // the +8 here is for high intensity
        // can also switch on color:
        // constexpr const char *const FG_COLOR[] = {
        //     "\033[30m",
        //     "\033[38;5;196m",
        //     "\033[38;5;148m",
        //     "\033[38;5;220m",
        //     "\033[38;5;21m",
        //     "\033[38;5;134m",
        //     "\033[36m",
        //     "\033[37m",
        //     "\033[39m"
        // };
    }
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
StyleString colored(std::string text, BetterColor fg = Color::None, BetterColor bg = Color::None);
void printStyle(StyleString str);

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

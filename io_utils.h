#pragma once
// #include <cctype>
// #include <vector>
#include "input.h"
#include "output.h"
#include <string>

// wait for enter key to be pressed
void wait_enter(my_io &io);
void typeOut(my_io &io, const StyleString &text, int sleepms = 18, int aftersleep = 0);
void typeOut(my_io &io, const char* text, int sleepms = 18, int aftersleep = 0);
void typeOutLine(my_io &io, const StyleString &text, int sleepms = 18, int aftersleep = 0);
void typeOutLine(my_io &io, const char* text, int sleepms = 18, int aftersleep = 0);
struct Option {
    std::string text;
    int key;
    Color color, selected_color;
    Option(const std::string &text_arg,
        int key_arg,
        Color color_arg = Color::White,
        Color selected_color_arg = Color::Yellow);
};

int optionsNav(my_io &io, const std::vector<Option> &options, std::string hint = "Select");

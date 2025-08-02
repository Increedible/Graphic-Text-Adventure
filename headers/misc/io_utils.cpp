// #include <cctype>
// #include "input.h"
// #include "output.h"
// #include <string>
#include <vector>
#include "utilities.h"
#include "io_utils.h"

BetterColor::BetterColor(int num) {
    value = num;
}
BetterColor::BetterColor(Color num) {
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

std::string BetterColor::toString() const{
    return "\033[38;5;" + std::to_string(value) + "m";
}

// string where every non-control character is seprated by \0

StyleString::StyleString(){}

// create StyleString with normal (no control character) string
StyleString::StyleString(const std::string &s){
    str = s;
}

StyleString::StyleString(const StyleString &ss){
    str = ss.str;
}

const char* StyleString::c_str() const{
    return str.c_str();
}

const char* StyleString::begin() const{
    return str.c_str();
}

const char* StyleString::end() const{
    return str.c_str() + str.size();
}

void StyleString::operator+=(const std::string &s){
    str += s;
}

void StyleString::operator+=(const StyleString &ss){
    str += ss.str;
}

void StyleString::push_back(const char&c) {
    str.push_back(c);
}

std::string StyleString::to_string() const{
    std::string s;
    for(auto c: s){
        if (c != '\0'){
            s.pop_back();
        }
    }
    return s;
}

std::string StyleString::to_raw_string() const{
    std::string s;
    for (const char *i=begin();i<end();i++){
        if (*i){
            s.push_back(*i);
        } else {
            for (i++;*i;i++);
        }
    }
    return s;
}

StyleString operator+(const StyleString &a, const StyleString &b) {
    StyleString ret;
    ret.str = a.str+b.str;
    return ret;
}

void StyleString::operator=(const StyleString& s) {
    str = s.str;
}

void StyleString::operator=(const std::string& s) {
    StyleString ret(s);
    str = ret.str;
}

StyleString operator+(const char* s, const StyleString &ss) {
    return StyleString(s + ss.str);
}

StyleString operator+(const StyleString &ss, const char* s) {
    return StyleString(ss.str + s);
}

StyleString operator+(const std::string &s, const StyleString &ss) {
    return StyleString(s + ss.str);
}

StyleString operator+(const StyleString &ss, const std::string &s) {
    return StyleString(ss.str + s);
}

// return string with color code
StyleString colored(const std::string &text, BetterColor fg, BetterColor bg) {
    StyleString ret;
    ret.push_back('\0');
    if (fg.value != -1) ret += fg.toString();
    if (bg.value != -1) ret += bg.toString();
    ret.push_back('\0');
    ret += text;
    ret.push_back('\0');
    ret += PIXEL_RESET;
    ret.push_back('\0');
    return ret;
}

void printStyle(const StyleString &str){
    for (const char *i = str.begin();i<str.end();i++) {
        i += printf("%s",i);
    }
}

// wait for enter key to be pressed
void wait_enter(MyIO &io) {
    do {
        io.check_sync();
    } while (!io.pressed[K_ENTER]);
}

void typeOut(MyIO &io, const StyleString &text, int sleepms, int aftersleep) {
    bool skip = 0;
    for (const char *i = text.begin();i<text.end();i++) {
        if (*i){
            printf("%c", *i);
            fflush(stdout);
            io.check();
            if (io.pressed[K_LEFT])skip = 1;
            if (*i == '\n')skip = 0;
            if (!skip) {
                MSDelay(sleepms);
            }
        } else {
            i++;
            i += printf("%s", i);
        }
    }
    printf("\n");
    fflush(stdout);
    MSDelay(aftersleep * 1000);
}

void typeOut(MyIO &io, const char* text, int sleepms, int aftersleep) {
    typeOut(io, StyleString(text), sleepms, aftersleep);
}

void typeOutLine(MyIO &io, const StyleString &text, int sleepms, int aftersleep) {
    bool skip = 0;
    for (const char*i=text.begin();i<text.end();i++) {
        if (*i){
            printf("%c", *i);
            if (*i != '\n')continue;
            fflush(stdout);
            io.check();
            if (io.pressed[K_LEFT])skip = 1;
            if (*i == '\n')skip = 0;
            if (!skip) {
                MSDelay(sleepms);
            }
        } else {
            i++;
            i += printf("%s", i);
        }
    }
    printf("\n");
    fflush(stdout);
    MSDelay(aftersleep * 1000);
}

void typeOutLine(MyIO &io, const char* text, int sleepms, int aftersleep) {
    typeOutLine(io, StyleString(text), sleepms, aftersleep);
}

Option::Option(const std::string &text_arg,
    int key_arg,
    Color color_arg,
    Color selected_color_arg):
    text(text_arg),
    key(key_arg),
    color(color_arg),
    selected_color(selected_color_arg) {}

int optionsNav(MyIO &io, const std::vector<Option> &options, std::string hint){
    while (hint.size()<8)hint.push_back(' ');
    int choice = 0, optionsCnt = options.size();
    bool rerender = true;
    //cout << "Use up and down arrow keys to navigate, right arrow key to pick." << endl;
    set_cursor(false);
    do {
        if (rerender) {
            rerender = false;
            for (int i=0;i<optionsCnt;i++){
                if (choice == i){
                    printStyle(colored(hint + "> " + options[i].text, options[i].selected_color));
                    printf("\n");
                } else {
                    printStyle(colored("          " + options[i].text, options[i].color));
                    printf("\n");
                }
            }
            goBack((options.size()));
            fflush(stdout);
        }
        io.check_sync();
        if (io.pressed[K_UP])
            if (choice > 0) {
                choice--;
                rerender = true;
            }
        if (io.pressed[K_DOWN])
            if (choice < (int)options.size() - 1) {
                choice++;
                rerender = true;
            }
    } while(!io.pressed[K_RIGHT]);
    for (int i = 0; i < (int)options.size(); i++)
        printf("\n");
    set_cursor(true);
    return options[choice].key;
}

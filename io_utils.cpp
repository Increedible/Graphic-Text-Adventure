// #include <cctype>
// #include "input.h"
// #include "output.h"
// #include <string>
#include <vector>
#include "utilities.h"
#include "io_utils.h"

// wait for enter key to be pressed
void wait_enter(my_io &io) {
    do {
        io.check_sync();
    } while (!io.pressed[K_ENTER]);
}

void typeOut(my_io &io, const StyleString &text, int sleepms, int aftersleep) {
    bool skip = 0;
    for (const char *i = text.str.c_str();*i;i++) {
        i += printf("%s",i);
        fflush(stdout);
        io.check();
        if (io.pressed[K_LEFT])skip = 1;
        if (*i == '\n')skip = 0;
        if (!skip) {
            MSDelay(sleepms);
        }
    }
    printf("\n");
    fflush(stdout);
    MSDelay(aftersleep * 1000);
}

void typeOut(my_io &io, const char* text, int sleepms, int aftersleep) {
    typeOut(io, StyleString(text), sleepms, aftersleep);
}

void typeOutLine(my_io &io, const StyleString &text, int sleepms, int aftersleep) {
    bool skip = 0;
    for (const char*i=text.str.c_str();*i;i++) {
        i += printf("%s",i);
        if (*i!='\n')continue;
        fflush(stdout);
        io.check();
        if (io.pressed[K_LEFT])skip = 1;
        if (!skip) {
            MSDelay(sleepms);
        }
    }
    printf("\n");
    fflush(stdout);
    MSDelay(aftersleep * 1000);
}

void typeOutLine(my_io &io, const char* text, int sleepms, int aftersleep) {
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

int optionsNav(my_io &io, const std::vector<Option> &options, std::string hint){
    while (hint.size()<8)hint.push_back(' ');
    int choice = 0, optionsCnt = options.size();
    bool rerender = true;
    //cout << "Use up and down arrow keys to navigate, right arrow key to pick." << endl;
    set_cursor(false);
    printf("\n");
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

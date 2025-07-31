#pragma once

// if exception should be throw when ctrl-c is pressed
#define THROW_ON_INT

#define K_INT 0x03
#define K_A 0x41
#define K_a 0x61
#define K_ENTER 0x80
#define K_UP 0x81
#define K_DOWN 0x82
#define K_RIGHT 0x83
#define K_LEFT 0x84

#define K_OTHER 0xff

#define KCOUNT 256

// exception throw when ctrl-c is pressed
class Interupt : public std::exception {
    public:
    const char* what () {
        return "Ctrl-c pressed";
    }
};

struct base_io {
    // key pressed since last check
    bool pressed[KCOUNT];
    void init();
    void uninit();
    void check();
    void check_sync();
};

#define K_A 0x41
#define K_a 0x61
#define K_ENTER 0x80
#define K_UP 0x81
#define K_DOWN 0x82
#define K_RIGHT 0x83
#define K_LEFT 0x84

#define K_OTHER 0xff

#define KCOUNT 256


struct base_io {
    // key pressed since last check
    bool pressed[KCOUNT];
    void init();
    void uninit();
    void check();
    void check_sync();
};
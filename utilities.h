#include <iostream>
#include <random>
#ifdef __linux__
    #include <unistd.h>
#else
    #include <windows.h>
#endif

void MSDelay(int ms) {
    #ifdef __linux__
        usleep(ms * 1000);
    #else
        Sleep(ms);
    #endif
}

mt19937 rng(time(0));
// [l, r)
int randomnum(int l, int r) {
    uniform_int_distribution gen(l, r-1);
    return gen(rng);
}
// [0,r)
int randomnum(int r) {
    return randomnum(0, r);
}
#pragma once
#ifdef _WIN32
    #include "input_windows.h"
#elif __linux__
    #include "input_linux.h"
#else
    #error "Unsupported platform"
#endif
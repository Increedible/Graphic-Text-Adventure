#pragma once
#include <windows.h>
#include <stdio.h>
#include "input_base.h"

struct MyIO : base_io{
    HANDLE hStdin;
    DWORD fdwSaveOldMode;

    // to remove the need of reallocating
    INPUT_RECORD irInBuf[128];
    char buf[8192];

    void init(){
        setvbuf(stdout, buf, _IOFBF, sizeof(buf));
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        if (hStdin == INVALID_HANDLE_VALUE){
            ErrorExit("GetStdHandle");
        }
        if (!GetConsoleMode(hStdin, &fdwSaveOldMode)){
            ErrorExit("GetConsoleMode");
        }
        DWORD fdwMode = ENABLE_WINDOW_INPUT;
        if (!SetConsoleMode(hStdin, fdwMode)){
            ErrorExit("SetConsoleMode");
        }
    }

    void ErrorExit (const char* lpszMessage){
        fprintf(stderr, "%s\n", lpszMessage);
        // Restore input mode on exit.
        SetConsoleMode(hStdin, fdwSaveOldMode);
        ExitProcess(1);
    }

    void check(){
        DWORD event_count;
        for (bool& i:pressed){
            i = 0;
        }
        do {
            if (!GetNumberOfConsoleInputEvents(hStdin, &event_count)){
                ErrorExit("CheckEventCount");
            }
            if (event_count > 0) {
                _check_sync();
            }
        } while (event_count > 0);
    }

    void check_sync(){
        for (bool& i:pressed){
            i = 0;
        }
        _check_sync();
    }

    // check but block
    void _check_sync(){
        DWORD cNumRead;
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        for (DWORD i=0;i<cNumRead;i++) {
            INPUT_RECORD& event = irInBuf[i];
            switch(event.EventType)
            {
                case KEY_EVENT: { // keyboard input
                    KEY_EVENT_RECORD& keyEvent = event.Event.KeyEvent;
                    if (keyEvent.bKeyDown) {
                        // key press
                        bool shift_pressed = keyEvent.dwControlKeyState & SHIFT_PRESSED;
                        bool capslock_on = keyEvent.dwControlKeyState & CAPSLOCK_ON;
                        bool lctrl_pressed = keyEvent.dwControlKeyState & LEFT_CTRL_PRESSED;
                        bool rctrl_pressed = keyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED;
                        bool ctrl_pressed = lctrl_pressed || rctrl_pressed;
                        int pressed_key;
                        switch (keyEvent.wVirtualKeyCode)
                        {
                        case VK_RETURN:
                            pressed_key = K_ENTER;
                            break;
                        case VK_LEFT:
                            pressed_key = K_LEFT;
                            break;
                        case VK_RIGHT:
                            pressed_key = K_RIGHT;
                            break;
                        case VK_UP:
                            pressed_key = K_UP;
                            break;
                        case VK_DOWN:
                            pressed_key = K_DOWN;
                            break;
                        case 'A':
                            if (shift_pressed ^ capslock_on) {
                                pressed_key = K_A;
                            } else {
                                pressed_key = K_a;
                            }
                            break;
                        case 'C':
                            if (ctrl_pressed){
                                #ifdef THROW_ON_INT
                                    throw Interupt();
                                #endif
                                pressed_key = K_INT;
                                break;
                            }
                            // break;
                        default:
                            pressed_key = K_OTHER;
                            break;
                        }
                        pressed[pressed_key] = 1;
                    } else {
                        // key release
                    }
                    break;
                }
                case MOUSE_EVENT: 
                case WINDOW_BUFFER_SIZE_EVENT:
                case FOCUS_EVENT:
                case MENU_EVENT:
                    break;
                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }

    void uninit(){
        SetConsoleMode(hStdin, fdwSaveOldMode);
    }
};

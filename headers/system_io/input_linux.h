#pragma once
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <stdlib.h>
// #include <fcntl.h>
// #include <cerrno>
#include "input_base.h"

struct MyIO : base_io{
    termios oldSettings;

    void init() {
        // // changing input mode
        termios newSettings;
        tcgetattr(fileno(stdin), &oldSettings);
        newSettings = oldSettings;
        newSettings.c_lflag &= (~ICANON & ~ECHO & ~ISIG);
        tcsetattr(fileno(stdin), TCSANOW, &newSettings);
        // set non-blocking
        // int flags = fcntl(fileno(stdin), F_GETFL, 0);
        // fcntl(fileno(stdin), F_SETFL, O_NONBLOCK | flags);
    }

    void error_exit(){
        tcsetattr(fileno(stdin),TCSANOW, &oldSettings);
        exit(1);
    }
    
    void uninit(){
        tcsetattr(fileno(stdin),TCSANOW, &oldSettings);
    }

    // read a single char
    void read_sync(char &c){
        int ret = read(fileno(stdin), &c, 1);
        if (ret == -1){
            perror("read");
            error_exit();
        } else if (ret==0){
            fprintf(stderr, "stdin closed unexpectly\n");
            error_exit();
        }
    }

    // return true if success
    bool read_inp(char &c){
        pollfd pfd;
        pfd.fd = fileno(stdin);
        pfd.events = POLLIN;
        int ret = poll(&pfd, 1, 0);
        if (ret == -1){
            perror("poll()");
            error_exit();
            return 0;
        } else if (ret == 0){
            return 0;
        } else {
            read_sync(c);
            return 1;
        }
    }

    void unknown_key(const char* prev = "", const char key = 0){
        fprintf(stderr, "unknown key: %s %02x\n", prev, key);
        error_exit();
    }

    void check() {
        for (bool&i:pressed)i=0;
        char c;
        while (read_inp(c)){
            switch (c)
            {
            case 0x03:
                #ifdef THROW_ON_INT
                    throw Interupt();
                #endif
                pressed[K_INT] = 1;
                break;
            case 0x0a:
                pressed[K_ENTER] = 1;
                break;
            case 0x1b:
                read_inp(c);
                switch (c)
                {
                case 0x5b:
                    read_inp(c);
                    switch (c)
                    {
                    case 0x41:
                        pressed[K_UP]=1;
                        break;
                    case 0x42:
                        pressed[K_DOWN]=1;
                        break;
                    case 0x43:
                        pressed[K_RIGHT]=1;
                        break;
                    case 0x44:
                        pressed[K_LEFT]=1;
                        break;
                    default:
                        unknown_key("1b 5b", c);
                        break;
                    }
                    break;
                default:
                    unknown_key("1b", c);
                    break;
                }
                break;
            case 0x41:
                pressed[K_A] = 1;
                break;
            case 0x61:
                pressed[K_a] = 1;
                break;
            default:
                unknown_key("", c);
                break;
            }
        }
    }

    void check_sync(){
        pollfd pfd;
        pfd.fd = fileno(stdin);
        pfd.events = POLLIN;
        int ret = poll(&pfd, 1, -1);
        if (ret == -1){
            perror("poll()");
            error_exit();
        } else {
            check();
        }
    }
};

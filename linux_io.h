#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
//#include <termios.h>
#include <cerrno>

#define K_ENTER 0
#define K_UP 1
#define K_DOWN 2
#define K_RIGHT 3
#define K_LEFT 4

#define KCOUNT 5

void init() {
    // // changing input mode
    // struct termios oldSettings, newSettings;

    // tcgetattr( fileno( stdin ), &oldSettings );
    // newSettings = oldSettings;
    // newSettings.c_lflag &= (~ICANON & ~ECHO);
    // tcsetattr( fileno( stdin ), TCSANOW, &newSettings );
    // int flags = fcntl(fileno(stdin), F_GETFL, 0);
    // fcntl(fileno(stdin), F_SETFL, O_NONBLOCK | flags);
}

int read_inp(char &c){
    return read(fileno(stdin), &c, 1);
}


// not supposed to happen
void unknown_key(){
    ;
}

void check(bool pressed[KCOUNT]){
    fflush(stdout);
    char c;
    for (int i=0;i<KCOUNT;i++)pressed[i]=0;
    for (int res = read_inp(c);res!=0;res=read_inp(c)){
        if (res==-1){
            if (errno != EAGAIN && errno != EWOULDBLOCK){
                perror("");
            }
            return;
        }
        switch (c)
        {
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
                    unknown_key();
                    break;
                }
                break;
            default:
                unknown_key();
                break;
            }
            break;
        default:
            break;
        }
    }
}


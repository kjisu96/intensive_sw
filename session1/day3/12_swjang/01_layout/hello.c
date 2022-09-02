#include <stdio.h>
//#define ALIGNED 1

#ifdef ALIGNED
struct foo {
    int     a;
    short   b;
    char    c;
    int     d;
    char    e;
    char    f;
    char    g;
    short   h;
};
#else
struct foo {
    int     a;
    int     d;
    short   b;
    short   h;
    char    c;
    char    e;
    char    f;
    char    g;
};
#endif

int main() {

    struct foo f1 = {
        0x12345678,
        0x12345678,
        0xFBCD,
        0xDDFF,
        0xEE,
        0xAB,
        0xCC,
        0xEE
    };

    printf("struct foo's size is %ld bytes\n", sizeof(f1));

    return 0;
}
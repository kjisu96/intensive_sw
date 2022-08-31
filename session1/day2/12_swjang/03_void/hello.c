// hello.c
#include <stdio.h>

int compare_int(int a, int b) {
    return (a==b);
}

int compare_float(float a, float b) {
    return (a==b);
}

int compare(int type, void* ap, void* bp) {
    if(type == 0) {
        int* a = (int*)ap;
        int* b = (int*)bp;
        return *a == *b;
    }
    else if(type == 1) {
        float* a = (float*)ap;
        float* b = (float*)bp;
        return *a == *b;
    }
}

int main() {
    int a = 100;
    int b = 200;
    //if(compare_int(a, b) == 1)
    if(compare(0, &a, &b) == 1)
        printf("a is equal to b\n");
    else
        printf("a is not equal to b\n");

    float c = 100.2;
    float d = 200.8;
    //if(compare_float(c, d) == 1)
    if(compare(1, &c, &d) == 1)
        printf("compare() : a is equal to b\n");
    else
        printf("compare() : a is not equal to b\n");

    return 0;
}


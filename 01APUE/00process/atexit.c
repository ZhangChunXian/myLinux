#include <stdio.h>

void func1() {
    printf("func1 is called\n");
}

void func2() {
    printf("func2 is callled\n");
}

void func3() {
    printf("func3 is called\n");
}

int main(int argc, char **argv) {
    atexit(func1);
    atexit(func2);
    atexit(func3);
    printf("process exit\n");
    return 0;
}
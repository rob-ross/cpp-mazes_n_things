#include <cstdio>
#include <ncurses.h>
#include <iostream>
#include <roblib/base.h>

void simple_mem_test() {

    // try malloc and new. These will trigger asan in debug build.
    int * i_ptr = static_cast<int *>(malloc(sizeof(int)));
    *i_ptr = 42;

    std::cout << "*i_ptr is " << *i_ptr << std::endl;
    free(i_ptr);

    // intentionally read out of array bounds
    int * some_ints = static_cast<int *>(malloc(sizeof(int) * 100));

    for ( int i = 0; i < 200; i++) {
        some_ints[i] = i; // half of these writes overflow this buffer.
    } // what will happen with asan??

    // also don't free(some_ints); on purpose
}

void smoke_test_n_curses() {
    initscr();
    erase();
    refresh();
    endwin();
}
int main() {
    printf("Hello from mazes_n_things!\n");

    // try using a macro from included header
    int x = 5; int y = 2;
    SWAP(x, y);

    // try using an ncurses api method
    smoke_test_n_curses();

    return 0;
}


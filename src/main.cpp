#include <cstdio>
#include <ncurses.h>
#include <iostream>
#include <roblib/base.h>

int main() {
    printf("Hello from mazes_n_things!\n");
    int x = 5; int y = 2;
    SWAP(x, y);

    // try malloc and new. These will trigger asan in debug build.
    int * i_ptr = static_cast<int *>(malloc(sizeof(int)));
    *i_ptr = 42;

    std::cout << "*i_ptr is " << *i_ptr << std::endl;
    free(i_ptr);



    return 0;
}

/*

Search paths:
/Users/robross/Documents/Development/CLionProjects/cpp-mazes_n_things/include
/Users/robross/Documents/Development/CLionProjects/cpp-mazes_n_things/cmake-build-debug/dependencies/c-roblib/install/include
/usr/local/Cellar/llvm/22.1.8/include/c++/v1
/usr/local/Cellar/llvm/22.1.8/lib/clang/22/include
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include
/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks


 */

#include <cstdio>

#include <ncurses.h>

#include <roblib/base.h>

int main() {
    printf("Hello from mazes_n_things!\n");
    int x = 5; int y = 2;
    SWAP(x, y);
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

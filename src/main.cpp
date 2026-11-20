//  main.cpp
//
//  Created by Rob Ross on 11/19/26.
//
//  Copyright (c) 2026.  All rights reserved.




// declares that there is a namespace `entry_point_1` and contains `int main();`
namespace entry_point_1 {
    // Namespace-scope function declarations have external linkage by default,
    // so `extern` here is already implied
   extern int main(int argc, const char * logfile_path);
}

// we can use the single main entry point to route based on command-line args
int main(int argc, char *argv[]) {
    const char * logfile_path = argc > 1 ? argv[1] : nullptr;
    int ret = entry_point_1::main(argc, logfile_path);

    return ret;
}


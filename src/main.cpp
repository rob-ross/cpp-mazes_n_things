

// declares that there is a namespace `entry_point_1` and contains `int main();`
namespace entry_point_1 {
    int main();
}

// we can use the single main entry point to route based on command-line args
int main() {

    int ret = entry_point_1::main();

    return 0;
}


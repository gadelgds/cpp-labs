#include <iostream>

int main() {
    char str[] = "Hello";
    const char* p = "Hello";

    std::cout << sizeof(str) << " " << sizeof(p) << std::endl;

    return 0;
}

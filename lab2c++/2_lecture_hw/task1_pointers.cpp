#include <iostream>

int main() {
    int a = 10;
    int b = 20;
    int* p = &a;
    *p = 30;
    p = &b;
    *p = 40;

    std::cout << a << " " << b << std::endl;

    return 0;
}

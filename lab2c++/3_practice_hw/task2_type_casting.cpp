#include <iostream>
#include <iomanip>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    int a = 7;
    int b = 3;

    // Вариант 1: Целочисленное деление
    int resInt = a / b;
    std::cout << "1. Целочисленное деление (a / b): " << resInt << std::endl;

    // Вариант 2: Явное приведение типов к double и округление до 3 знаков
    double resDouble = static_cast<double>(a) / b;
    std::cout << "2. Деление с явным приведением к double (3 знака): ";
    std::cout << std::fixed << std::setprecision(3) << resDouble << std::endl;

    return 0;
}

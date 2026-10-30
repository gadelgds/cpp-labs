#include <iostream>
#include <iomanip>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== НАИЗУСТЬ: ЯВНОЕ ПРИВЕДЕНИЕ ТИПОВ ЧЕРЕЗ static_cast ===" << std::endl;

    // ЧАСТЬ 1: Вычитание int из long с явным приведением к long
    long valLong = 100000L;
    int valInt = 25000;

    // Вычитание с явным приведением static_cast<long>()
    long resultLong = valLong - static_cast<long>(valInt);

    std::cout << "\n[Целочисленное вычитание]:" << std::endl;
    std::cout << "  Переменная long:                 " << valLong << std::endl;
    std::cout << "  Переменная int:                  " << valInt << std::endl;
    std::cout << "  Результат (long - static_cast<long>(int)): " << resultLong << std::endl;

    // ЧАСТЬ 2: Вычитание float из double с явным приведением к double
    double valDouble = 125.75;
    float valFloat = 25.25f;

    // Вычитание с явным приведением static_cast<double>()
    double resultDouble = valDouble - static_cast<double>(valFloat);

    std::cout << "\n[Вещественное вычитание с плавающей точкой]:" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  Переменная double:               " << valDouble << std::endl;
    std::cout << "  Переменная float:                " << valFloat << std::endl;
    std::cout << "  Результат (double - static_cast<double>(float)): " << resultDouble << std::endl;

    return 0;
}

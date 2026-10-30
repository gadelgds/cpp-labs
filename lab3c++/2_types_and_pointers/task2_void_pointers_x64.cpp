#include <iostream>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== САМОСТОЯТЕЛЬНО 1: УКАЗАТЕЛИ void* И РАЗМЕРЫ В x64 ===" << std::endl;

    // 1. Создание переменных типов int и long со значениями
    int numInt = 42;
    long numLong = 123456L;

    // 2. Создание независимых универсальных указателей типа void*
    void* ptrVoid1 = &numInt;
    void* ptrVoid2 = &numLong;

    // 3. Обратное преобразование указателей void* в исходные типы int* и long*
    int* ptrInt = static_cast<int*>(ptrVoid1);
    long* ptrLong = static_cast<long*>(ptrVoid2);

    // 4. Извлечение значений из памяти (разыменование) и вывод на экран
    std::cout << "\n[Извлеченные значения]:" << std::endl;
    std::cout << "  Значение из ptrInt:  " << *ptrInt << " (адрес: " << ptrInt << ")" << std::endl;
    std::cout << "  Значение из ptrLong: " << *ptrLong << " (адрес: " << ptrLong << ")" << std::endl;

    // 5. Определение размеров указателей и типов в архитектуре сборки (x64)
    std::cout << "\n[Размеры типов и указателей в текущей системе]:" << std::endl;
    std::cout << "  Разрядность сборки:    " << (sizeof(void*) * 8) << "-бит" << std::endl;
    std::cout << "  sizeof(int):           " << sizeof(int) << " байт (" << sizeof(int) * 8 << " бит)" << std::endl;
    std::cout << "  sizeof(long):          " << sizeof(long) << " байт (" << sizeof(long) * 8 << " бит)" << std::endl;
    std::cout << "  sizeof(long long):     " << sizeof(long long) << " байт (" << sizeof(long long) * 8 << " бит)" << std::endl;
    std::cout << "  sizeof(void*):         " << sizeof(void*) << " байт (" << sizeof(void*) * 8 << " бит)" << std::endl;
    std::cout << "  sizeof(int*):          " << sizeof(int*) << " байт (" << sizeof(int*) * 8 << " бит)" << std::endl;
    std::cout << "  sizeof(long*):         " << sizeof(long*) << " байт (" << sizeof(long*) * 8 << " бит)" << std::endl;

    return 0;
}

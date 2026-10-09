#include <windows.h>
#include <iostream>
#include <iomanip>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== Исследование моделей данных C++ и Win32 ===" << std::endl;

    #if defined(_WIN64)
    std::cout << "Архитектура: x64 (64-битная система)" << std::endl;
    #elif defined(_WIN32)
    std::cout << "Архитектура: x86 (32-битная система)" << std::endl;
    #endif

    std::cout << "\n[Базовые типы C++]:" << std::endl;
    std::cout << "sizeof(int):       " << sizeof(int)       << " байт (" << sizeof(int) * 8       << " бит)" << std::endl;
    std::cout << "sizeof(long):      " << sizeof(long)      << " байт (" << sizeof(long) * 8      << " бит)" << std::endl;
    std::cout << "sizeof(long long): " << sizeof(long long) << " байт (" << sizeof(long long) * 8 << " бит)" << std::endl;
    std::cout << "sizeof(void*):     " << sizeof(void*)     << " байт (" << sizeof(void*) * 8     << " бит)" << std::endl;

    std::cout << "\n[Типы Win32 API]:" << std::endl;
    std::cout << "sizeof(DWORD):     " << sizeof(DWORD)     << " байт (" << sizeof(DWORD) * 8     << " бит)" << std::endl;
    std::cout << "sizeof(LONG):      " << sizeof(LONG)      << " байт (" << sizeof(LONG) * 8      << " бит)" << std::endl;
    std::cout << "sizeof(ULONG_PTR): " << sizeof(ULONG_PTR) << " байт (" << sizeof(ULONG_PTR) * 8 << " бит)" << std::endl;
    std::cout << "sizeof(DWORD_PTR): " << sizeof(DWORD_PTR) << " байт (" << sizeof(DWORD_PTR) * 8 << " бит)" << std::endl;
    std::cout << "sizeof(INT_PTR):   " << sizeof(INT_PTR)   << " байт (" << sizeof(INT_PTR) * 8   << " бит)" << std::endl;

    return 0;
}

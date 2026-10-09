/**
 * @file task7_win32_types_sizeof.cpp
 * @brief Задача 7 (Ворд, Задание 2): Исследование размеров типов C++ и Win32 (x86 vs x64).
 * 
 * УСЛОВИЕ:
 * Написать программу, которая выводит через sizeof() размеры базовых типов C++ 
 * (int, long, long long, void*) и типов Win32 (DWORD, LONG, ULONG_PTR, DWORD_ptr, INT_PTR).
 * Скомпилировать программу в конфигурациях x86 и x64.
 * Записать результаты в таблицу и объяснить разницу в поведении типов.
 * 
 * СВОДНАЯ ТАБЛИЦА РАЗМЕРОВ ТИПОВ:
 * +-------------------+-------------+-------------+----------------------------------------------+
 * | Тип данных        | x86 (32-bit)| x64 (64-bit)| Описание и назначение                        |
 * +-------------------+-------------+-------------+----------------------------------------------+
 * | int               | 4 байта     | 4 байта     | Базовый целочисленный тип C++                |
 * | long              | 4 байта     | 4 байта     | В Windows модель LLP64: long всегда 32 бита  |
 * | long long         | 8 байт      | 8 байт      | 64-битное целое число                        |
 * | void*             | 4 байта     | 8 байт      | Указатель на память (адресное пространство)  |
 * +-------------------+-------------+-------------+----------------------------------------------+
 * | DWORD             | 4 байта     | 4 байта     | unsigned long (всегда 32 бита без знака)     |
 * | LONG              | 4 байта     | 4 байта     | long (всегда 32 бита со знаком)              |
 * | ULONG_PTR         | 4 байта     | 8 байт      | Полиморфный беззнаковый int размера pointer  |
 * | DWORD_PTR         | 4 байта     | 8 байт      | Полиморфный int для хранения указателей      |
 * | INT_PTR           | 4 байта     | 8 байт      | Полиморфный знаковый int размера pointer     |
 * +-------------------+-------------+-------------+----------------------------------------------+
 * 
 * ОБЪЯСНЕНИЕ РАЗНИЦЫ В ПОВЕДЕНИИ ТИПОВ (МОДЕЛИ ПАМЯТИ):
 * 1. Архитектура Windows x64 использует модель данных LLP64:
 *    - 'int' и 'long' остаются 32-битными (4 байта) для обеспечения обратной 
 *      совместимости с миллионами строк существующего кода 32-битной эры Win32.
 *    - Только указатели (void*, char*...) и 'long long' становятся 64-битными (8 байт).
 *    (Для сравнения: в Linux x64 используется модель LP64, где 'long' равен 8 байтам).
 * 
 * 2. Роль типов с суффиксом *_PTR (Polymorphic Data Types):
 *    - ULONG_PTR, DWORD_PTR, INT_PTR — это НЕ указатели (pointer), это ЦЕЛЫЕ ЧИСЛА!
 *    - Суффикс '_PTR' означает: "целое число с точностью до указателя" (pointer-precision integer).
 *    - На x86 они определены как 32-битные целые (unsigned long / int).
 *    - На x64 они расширяются до 64-битных целых (unsigned __int64 / __int64).
 *    - Они критически необходимы для безопасного сохранения адресов или хэндлов в виде целых чисел:
 *      на x64 каст указателя в DWORD приведет к потере старших 32 бит, а каст в DWORD_PTR безопасен!
 */

#include <windows.h>
#include <iostream>
#include <iomanip>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "===========================================================" << std::endl;
    std::cout << "      ИССЛЕДОВАНИЕ МОДЕЛЕЙ ДАННЫХ И РАЗМЕРОВ ТИПОВ         " << std::endl;
    std::cout << "===========================================================" << std::endl;

    // Определение текущей архитектуры при компиляции
    #if defined(_WIN64)
    std::cout << "Текущая запущенная конфигурация: x64 (64-битная сборка)" << std::endl;
    #elif defined(_WIN32)
    std::cout << "Текущая запущенная конфигурация: x86 (32-битная сборка)" << std::endl;
    #endif

    std::cout << "\n--- РАЗМЕРЫ БАЗОВЫХ ТИПОВ C++ ---" << std::endl;
    std::cout << std::left << std::setw(20) << "Тип" 
              << std::setw(15) << "Размер (байт)" 
              << "Размер (бит)" << std::endl;
    std::cout << std::string(45, '-') << std::endl;
    std::cout << std::setw(20) << "int"       << std::setw(15) << sizeof(int)       << sizeof(int) * 8 << std::endl;
    std::cout << std::setw(20) << "long"      << std::setw(15) << sizeof(long)      << sizeof(long) * 8 << std::endl;
    std::cout << std::setw(20) << "long long" << std::setw(15) << sizeof(long long) << sizeof(long long) * 8 << std::endl;
    std::cout << std::setw(20) << "void*"     << std::setw(15) << sizeof(void*)     << sizeof(void*) * 8 << std::endl;

    std::cout << "\n--- РАЗМЕРЫ ТИПОВ WIN32 API ---" << std::endl;
    std::cout << std::left << std::setw(20) << "Тип Win32" 
              << std::setw(15) << "Размер (байт)" 
              << "Размер (бит)" << std::endl;
    std::cout << std::string(45, '-') << std::endl;
    std::cout << std::setw(20) << "DWORD"     << std::setw(15) << sizeof(DWORD)     << sizeof(DWORD) * 8 << std::endl;
    std::cout << std::setw(20) << "LONG"      << std::setw(15) << sizeof(LONG)      << sizeof(LONG) * 8 << std::endl;
    std::cout << std::setw(20) << "ULONG_PTR" << std::setw(15) << sizeof(ULONG_PTR) << sizeof(ULONG_PTR) * 8 << std::endl;
    std::cout << std::setw(20) << "DWORD_PTR" << std::setw(15) << sizeof(DWORD_PTR) << sizeof(DWORD_PTR) * 8 << std::endl;
    std::cout << std::setw(20) << "INT_PTR"   << std::setw(15) << sizeof(INT_PTR)   << sizeof(INT_PTR) * 8 << std::endl;

    std::cout << "\n--- АНАЛИТИЧЕСКИЕ ВЫВОДЫ ---" << std::endl;
    std::cout << "1. Типы int, long, DWORD, LONG НЕ изменили размер (всегда 4 байта = 32 бита)." << std::endl;
    std::cout << "2. Типы void*, ULONG_PTR, DWORD_PTR, INT_PTR масштабируются вместе с архитектурой:" << std::endl;
    std::cout << "   - в x86: 4 байта (32 бита)" << std::endl;
    std::cout << "   - в x64: 8 байт  (64 бита)" << std::endl;
    std::cout << "3. Это гарантирует, что типы *_PTR безопасны для хранения адресов указателей!" << std::endl;

    return 0;
}

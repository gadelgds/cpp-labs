/**
 * @file task8_ptr_truncation_fix.cpp
 * @brief Задача 8 (Ворд, Задание 3): Исправление ошибки усечения указателя (Pointer Truncation) в x64.
 * 
 * ИСХОДНЫЙ КОД С ОШИБКОЙ:
 * -------------------------------------------------------------
 * void ProcessData(DWORD dataStylePointer) {
 *     int* pNumber = (int*)dataStylePointer;
 *     if (pNumber != nullptr) {
 *         std::cout << "Value: " << *pNumber << std::endl;
 *     }
 * }
 * int main() {
 *     int criticalValue = 2026;
 *     int* lpContext = &criticalValue;
 *     DWORD TRUNCATED_PTR = (DWORD)lpContext; // ОШИБКА В x64!
 *     ProcessData(TRUNCATED_PTR);
 *     return 0;
 * }
 * -------------------------------------------------------------
 * 
 * В ЧЕМ ЗАКЛЮЧАЕТСЯ ПРОБЛЕМА:
 * 1. В конфигурации x86 (32-бита):
 *    - Размер указателя int* равен 4 байтам (32 бита).
 *    - Размер типа DWORD равен 4 байтам (32 бита).
 *    - Приведение (DWORD)lpContext не теряет данные, программа успешно работает.
 * 
 * 2. В конфигурации x64 (64-бита):
 *    - Размер указателя int* равен 8 байтам (64 бита, адрес вида 0x00007FF7ABCD1234).
 *    - Размер типа DWORD равен 4 байтам (32 бита).
 *    - При выполнении (DWORD)lpContext компилятор выдает предупреждение/ошибку:
 *      warning C4311: 'type cast': pointer truncation from 'int *' to 'DWORD'
 *      (или -Wpointer-to-int-cast в GCC/Clang).
 *    - СТАРШИЕ 32 БИТА АДРЕСА ОБРЕЗАЮТСЯ И ТЕРЯЮТСЯ!
 *    - В функции ProcessData обратный каст (int*)dataStylePointer восстанавливает
 *      только младшие 32 бита (адрес становится вида 0x00000000ABCD1234).
 *    - Разыменование такого адреса (*pNumber) приводит к мгновенному падению программы:
 *      Access Violation (0xC0000005) / Segmentation Fault!
 * 
 * КАК ИСПРАВИТЬ:
 * Использовать полиморфный целочисленный тип Windows API: DWORD_PTR или ULONG_PTR
 * (либо стандартный кроссплатформенный uintptr_t из <cstdint>).
 * Тип DWORD_PTR автоматически равен 32 битам в x86 и 64 битам в x64, поэтому
 * ни один бит адреса не теряется!
 */

#include <windows.h>
#include <iostream>
#include <iomanip>

// ИСПРАВЛЕННАЯ ФУНКЦИЯ: принимаем DWORD_PTR вместо DWORD
void ProcessData(DWORD_PTR dataStylePointer) {
    int* pNumber = reinterpret_cast<int*>(dataStylePointer);
    if (pNumber != nullptr) {
        std::cout << "[ProcessData] Успешно прочитано значение по адресу " 
                  << (void*)pNumber << " -> Value: " << *pNumber << std::endl;
    }
}

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ЗАДАЧА 8: Исправление усечения указателей в x86/x64 ===" << std::endl;

    int criticalValue = 2026;
    int* lpContext = &criticalValue;

    std::cout << "\n[Анализ указателя и памяти]:" << std::endl;
    std::cout << "  Реальный адрес переменной lpContext: 0x" 
              << std::hex << (UINT_PTR)lpContext << std::dec << std::endl;
    std::cout << "  Размер указателя lpContext:          " << sizeof(lpContext) << " байт" << std::endl;
    std::cout << "  Размер DWORD:                        " << sizeof(DWORD) << " байт" << std::endl;
    std::cout << "  Размер DWORD_PTR:                    " << sizeof(DWORD_PTR) << " байт" << std::endl;

    // Демонстрация того, что произошло бы при усечении через DWORD
    DWORD truncated = (DWORD)(UINT_PTR)lpContext;
    std::cout << "\n[Что произойдет при касте в DWORD (32 бита)]:" << std::endl;
    std::cout << "  Значение DWORD TRUNCATED_PTR:        0x" << std::hex << truncated << std::dec << std::endl;
    if (sizeof(void*) > sizeof(DWORD)) {
        std::cout << "  ВНИМАНИЕ: Произошло усечение адреса! Старшие 32 бита потеряны!" << std::endl;
    } else {
        std::cout << "  (В 32-битной сборке размеры совпадают, потеря бит не возникла)" << std::endl;
    }

    // ИСПРАВЛЕННОЕ БЕЗОПАСНОЕ РЕШЕНИЕ:
    std::cout << "\n[Безопасное решение через DWORD_PTR]:" << std::endl;
    DWORD_PTR SAFE_PTR = (DWORD_PTR)lpContext;
    std::cout << "  Значение DWORD_PTR SAFE_PTR:         0x" << std::hex << SAFE_PTR << std::dec << std::endl;
    
    // Передаем безопасный указатель в функцию
    ProcessData(SAFE_PTR);

    return 0;
}

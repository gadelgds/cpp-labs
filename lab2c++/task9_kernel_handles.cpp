/**
 * @file task9_kernel_handles.cpp
 * @brief Задача 9 (Ворд, Задание 4): Исследование дескрипторов объектов ядра Windows (HANDLE).
 * 
 * УСЛОВИЕ:
 * Написать программу, которая открывает три разных объекта ядра Windows: 
 * например, создает/открывает файл (CreateFile), создает событие (CreateEvent) 
 * и получает хэндл текущего процесса (GetCurrentProcess). 
 * Вывести их численные значения на экран (приведя к void* или UINT_PTR).
 * Объяснить:
 * 1. Почему значения хэндлов часто кратны 4 и похожи на малые числа?
 * 2. Почему GetCurrentProcess возвращает псевдо-хэндл -1 (0xFFFFFFFF)?
 * 
 * ТЕОРЕТИЧЕСКИЕ ОТВЕТЫ:
 * 
 * 1. ПОЧЕМУ ХЭНДЛЫ КРАТНЫ 4 И ПОХОЖИ НА МАЛЫЕ ЧИСЛА?
 *    - В Windows дескриптор (HANDLE) — это НЕ прямой указатель на структуру данных ядра
 *      в памяти ядра (Kernel Space), так как передавать прямые адреса ядра в User Mode
 *      небезопасно.
 *    - Дескриптор представляет собой индекс (смещение) в системной таблице дескрипторов
 *      процесса (Process Handle Table), управляемой ядром Windows Executive.
 *    - Внутренне таблица организована как массив структур дескрипторов (каждая запись
 *      хранит указатель на объект ядра и маску прав доступа).
 *    - Размер элемента или шаг смещения в таблице кратен 4 байтам, а младшие 2 бита
 *      числа зарезервированы ядром под служебные флаги (например, флаг защиты дескриптора).
 *    - Поэтому индексы нумеруются со смещением: 4, 8, 12, 16, 20, 24... (0x4, 0x8, 0xC, 0x10...).
 *      Они являются малыми целыми числами, строго кратными 4.
 * 
 * 2. ПОЧЕМУ GetCurrentProcess() ВОЗВРАЩАЕТ ПСЕВДО-ХЭНДЛ -1 (0xFFFFFFFF)?
 *    - GetCurrentProcess() возвращает константу ((HANDLE)(LONG_PTR)-1), то есть 0xFFFFFFFF в x86
 *      или 0xFFFFFFFFFFFFFFFF в x64. Это так называемый "псевдо-дескриптор" (pseudo-handle).
 *    - Он НЕ создает новую запись в таблице дескрипторов процесса и НЕ увеличивает
 *      счетчик ссылок на объект процесса.
 *    - Для системных функций Windows (например, WaitForSingleObject, SetPriorityClass) значение -1
 *      служит быстрым сигналом: "запрос адресован к вызывающему процессу".
 *    - Псевдо-дескриптор НЕ НУЖНО (и бессмысленно) закрывать через CloseHandle().
 *    - Если нужен НАСТОЯЩИЙ хэндл процесса (например, чтобы передать его в другой процесс),
 *      его получают через DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), ...).
 */

#include <windows.h>
#include <iostream>
#include <iomanip>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "===========================================================" << std::endl;
    std::cout << "       ИССЛЕДОВАНИЕ ХЭНДЛОВ ОБЪЕКТОВ ЯДРА WINDOWS          " << std::endl;
    std::cout << "===========================================================" << std::endl;

    // 1. Создание/открытие файла через CreateFile
    HANDLE hFile = CreateFileW(
        L"lab2_kernel_demo.tmp",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
        NULL
    );

    // 2. Создание события через CreateEvent
    HANDLE hEvent = CreateEventW(
        NULL,   // Атрибуты безопасности по умолчанию
        TRUE,   // Ручной сброс (Manual-reset)
        FALSE,  // Начальное состояние — несигнальное
        NULL    // Безымянное событие
    );

    // 3. Получение псевдо-дескриптора текущего процесса
    HANDLE hProcessPseudo = GetCurrentProcess();

    // 4. Получение РЕАЛЬНОГО дескриптора текущего процесса через DuplicateHandle для сравнения
    HANDLE hProcessReal = NULL;
    DuplicateHandle(
        GetCurrentProcess(),
        GetCurrentProcess(),
        GetCurrentProcess(),
        &hProcessReal,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS
    );

    // ВЫВОД ЗНАЧЕНИЙ НА ЭКРАН
    std::cout << "\n[Таблица значений дескрипторов]:" << std::endl;
    std::cout << std::left << std::setw(28) << "Объект ядра" 
              << std::setw(18) << "Значение (void*)" 
              << std::setw(18) << "Число (UINT_PTR)" 
              << "Кратность 4?" << std::endl;
    std::cout << std::string(75, '-') << std::endl;

    auto printHandle = [](const char* name, HANDLE h, bool isPseudo = false) {
        UINT_PTR val = (UINT_PTR)h;
        bool mult4 = (!isPseudo && (val % 4 == 0));
        std::cout << std::left << std::setw(28) << name 
                  << std::setw(18) << (void*)h 
                  << std::dec << std::setw(18) << val 
                  << (isPseudo ? "Псевдо (-1)" : (mult4 ? "Да (кратно 4)" : "Нет")) 
                  << std::endl;
    };

    printHandle("1. Файл (CreateFile)", hFile);
    printHandle("2. Событие (CreateEvent)", hEvent);
    printHandle("3. Процесс (GetCurrentProcess)", hProcessPseudo, true);
    if (hProcessReal) {
        printHandle("4. Процесс (Реальный хэндл)", hProcessReal);
    }

    std::cout << "\n[ПОДРОБНЫЙ РАЗБОР]:" << std::endl;
    std::cout << "1. Численные значения hFile и hEvent — это малые целые числа (кратные 4)." << std::endl;
    std::cout << "   Они представляют собой смещения записей в таблице дескрипторов процесса." << std::endl;
    std::cout << "2. GetCurrentProcess() вернул: " << (void*)hProcessPseudo 
              << " (" << (INT_PTR)hProcessPseudo << ") — это константа-маркер для быстрого вызова без поиска в таблице." << std::endl;
    std::cout << "3. Реальный дескриптор процесса (через DuplicateHandle): " 
              << (void*)hProcessReal << " уже является обычной записью и кратен 4!" << std::endl;

    // Корректное закрытие реальных дескрипторов
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
    if (hEvent != NULL) {
        CloseHandle(hEvent);
    }
    if (hProcessReal != NULL) {
        CloseHandle(hProcessReal);
    }
    // Внимание: CloseHandle(hProcessPseudo) НЕ вызывается, так как псевдо-хэндл не закрывается!

    std::cout << "\nВсе реальные дескрипторы успешно закрыты через CloseHandle()." << std::endl;

    return 0;
}

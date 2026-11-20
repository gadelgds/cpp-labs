// Лабораторная работа №8, задание 1.
// Краткоживущие потоки: CreateThread против _beginthreadex.
// Сборка MSVC:  cl /EHsc task1_createthread_vs_beginthreadex.cpp psapi.lib
// Сборка MinGW: g++ -o task1.exe task1_createthread_vs_beginthreadex.cpp -lpsapi
// Запуск: task1.exe           — вариант CreateThread (по умолчанию)
//        task1.exe create     — то же, явно
//        task1.exe begin      — вариант _beginthreadex

#include <windows.h>
#include <process.h>   // _beginthreadex
#include <psapi.h>     // GetProcessMemoryInfo
#include <iostream>
#include <cstring>     // strtok, strcpy

// Сколько потоков создать всего.
const int THREAD_COUNT = 10000;
// Сколько раз strtok вызывается внутри одного потока.
const int STRTOK_CALLS = 100;

// Тестовая строка для strtok. strtok портит строку,
// поэтому каждый поток работает со своей копией.
const char* TEST_STRING = "один,два,три;четыре пять:шесть";

// Функция потока: многократно разбирает копию тестовой строки.
DWORD WINAPI WorkerThread(LPVOID /* param */)
{
    char buffer[128];
    for (int i = 0; i < STRTOK_CALLS; ++i) {
        // Свежая копия: strtok ставит '\0' вместо разделителей.
        // Буфер 128 байт, тестовая строка намного короче.
        strcpy(buffer, TEST_STRING);
        // Первый вызов — с указателем на строку.
        char* token = strtok(buffer, ",;: ");
        // Дальше — с NULL: продолжаем разбор той же строки.
        while (token != nullptr) {
            token = strtok(nullptr, ",;: ");
        }
    }
    return 0;
}

// Печатает текущее потребление памяти процесса.
// PrivateUsage — память, занятая только этим процессом (то, что
// видно в диспетчере задач как Private Bytes в столбце "Память").
// WorkingSetSize — память, реально находящаяся в ОЗУ сейчас.
void PrintMemory(const char* label)
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        std::cout << label
                  << "  WorkingSet: " << pmc.WorkingSetSize / 1024 << " КБ"
                  << "  PrivateUsage: " << pmc.PrivateUsage / 1024 << " КБ"
                  << std::endl;
    }
    else {
        std::cout << label << "  (не удалось прочитать память, код "
                  << GetLastError() << ")" << std::endl;
    }
}

int main(int argc, char* argv[])
{
#if defined(_WIN32)
    system("chcp 65001 > nul"); // Русский текст в консоли без кракозябр.
#endif

    // Выбор варианта по аргументу командной строки.
    bool useBeginThreadEx = (argc > 1 && strcmp(argv[1], "begin") == 0);

    std::cout << "=== ЗАДАНИЕ 1: CreateThread против _beginthreadex ===" << std::endl;
    std::cout << "Вариант: "
              << (useBeginThreadEx ? "_beginthreadex" : "CreateThread")
              << std::endl;
    std::cout << "Потоков к созданию: " << THREAD_COUNT << std::endl;
    PrintMemory("Старт:   ");

    for (int i = 0; i < THREAD_COUNT; ++i) {
        HANDLE hThread = nullptr;

        if (useBeginThreadEx) {
            // _beginthreadex: поток с инициализацией CRT.
            // Сигнатура WorkerThread приводится к типу _beginthreadex.
            uintptr_t h = _beginthreadex(
                nullptr, 0,
                reinterpret_cast<unsigned (__stdcall*)(void*)>(WorkerThread),
                nullptr, 0, nullptr);
            hThread = reinterpret_cast<HANDLE>(h);
        }
        else {
            // CreateThread: «голый» поток Windows без CRT-состояния.
            hThread = CreateThread(
                nullptr, 0, WorkerThread, nullptr, 0, nullptr);
        }

        if (hThread == nullptr) {
            std::cout << "Ошибка создания потока №" << i
                      << ", код " << GetLastError() << std::endl;
            return 1;
        }

        // Ждём завершения потока и закрываем его дескриптор.
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);

        // Каждую тысячу потоков — срез памяти, чтобы видеть рост.
        if ((i + 1) % 1000 == 0) {
            std::cout << "Создано потоков: " << (i + 1) << "  ";
            PrintMemory("");
        }
    }

    std::cout << "Создано потоков всего: " << THREAD_COUNT << std::endl;
    PrintMemory("Финиш:  ");
    std::cout << "Сравните PrivateUsage на старте и на финише." << std::endl;
    std::cout << "С CreateThread PrivateUsage заметно растёт, "
              << "с _beginthreadex — почти не меняется." << std::endl;

    return 0;
}

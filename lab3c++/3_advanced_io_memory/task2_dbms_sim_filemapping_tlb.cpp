#include <windows.h>
#include <iostream>
#include <vector>
#include <random>

const DWORD FILE_SIZE_16MB = 16 * 1024 * 1024; // 16 МБ
const int NUM_OPERATIONS = 10000;              // 10 000 случайных модификаций

// Создание 16 МБ бинарного файла последовательности int
bool CreateDbBinaryFile(const wchar_t* path) {
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    const size_t intsPerBlock = 64 * 1024;
    std::vector<int> block(intsPerBlock);
    for (size_t i = 0; i < intsPerBlock; ++i) block[i] = static_cast<int>(i);

    DWORD totalInts = FILE_SIZE_16MB / sizeof(int);
    DWORD writtenInts = 0;
    while (writtenInts < totalInts) {
        DWORD toWriteInts = min(intsPerBlock, totalInts - writtenInts);
        DWORD writtenBytes = 0;
        WriteFile(hFile, block.data(), toWriteInts * sizeof(int), &writtenBytes, NULL);
        writtenInts += toWriteInts;
    }
    CloseHandle(hFile);
    return true;
}

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    const wchar_t* dbFile = L"database_16mb.bin";
    std::wcout << L"=== ПРАКТИКА №2: СИМУЛЯЦИЯ СУБД (СТАНДАРТНЫЙ I/O vs FILE MAPPING & TLB FLUSH) ===" << std::endl;

    std::wcout << L"1. Создание бинарного файла 16 МБ с массивом целых чисел (int)..." << std::endl;
    CreateDbBinaryFile(dbFile);

    // Генерация массива случайных индексов для честного сравнения одинаковых обращений
    std::mt19937 rng(1337);
    DWORD totalIntCount = FILE_SIZE_16MB / sizeof(int);
    std::vector<DWORD> randomIndices(NUM_OPERATIONS);
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        randomIndices[i] = rng() % totalIntCount;
    }

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    // ==============================================================
    // РЕЖИМ А: СТАНДАРТНЫЙ I/O (SetFilePointerEx + ReadFile / WriteFile)
    // ==============================================================
    std::wcout << L"\n2. Запуск Режима А (Стандартный I/O со системным кэшем): 10 000 операций..." << std::endl;
    HANDLE hFileA = CreateFileW(dbFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    QueryPerformanceCounter(&start);
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        LARGE_INTEGER offset;
        offset.QuadPart = (LONGLONG)randomIndices[i] * sizeof(int);

        SetFilePointerEx(hFileA, offset, NULL, FILE_BEGIN);
        int value = 0;
        DWORD bytesRead = 0;
        ReadFile(hFileA, &value, sizeof(int), &bytesRead, NULL);

        value += 1; // Модификация записи

        SetFilePointerEx(hFileA, offset, NULL, FILE_BEGIN);
        DWORD bytesWritten = 0;
        WriteFile(hFileA, &value, sizeof(int), &bytesWritten, NULL);
    }
    QueryPerformanceCounter(&end);
    CloseHandle(hFileA);

    double timeModeA = static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    std::wcout << L"   -> Время выполнения Режима А: " << timeModeA << L" мс" << std::endl;

    // ==============================================================
    // РЕЖИМ Б: СКОЛЬЗЯЩЕЕ ОКНО MapViewOfFile + UnmapViewOfFile (TLB FLUSH)
    // ==============================================================
    std::wcout << L"\n3. Запуск Режима Б (Ручное проецирование с UnmapViewOfFile): 10 000 операций..." << std::endl;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD allocGranularity = si.dwAllocationGranularity; // 64 КБ
    DWORD windowSize = allocGranularity;                 // Скользящее окно 64 КБ

    HANDLE hFileB = CreateFileW(dbFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hMappingB = CreateFileMappingW(hFileB, NULL, PAGE_READWRITE, 0, 0, NULL);

    QueryPerformanceCounter(&start);
    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        LONGLONG byteOffset = (LONGLONG)randomIndices[i] * sizeof(int);

        // Выравнивание смещения на границу 64 КБ
        DWORD alignedOffsetLow = (DWORD)((byteOffset / allocGranularity) * allocGranularity);
        DWORD offsetWithinWindow = (DWORD)(byteOffset % allocGranularity);

        // Проецирование участка файла
        char* pView = static_cast<char*>(MapViewOfFile(hMappingB, FILE_MAP_ALL_ACCESS, 0, alignedOffsetLow, windowSize));
        if (pView) {
            int* pVal = reinterpret_cast<int*>(pView + offsetWithinWindow);
            *pVal += 1; // Модификация в спроецированной памяти

            // Принудительное размонтирование (инвалидация записей в TLB процессора)
            UnmapViewOfFile(pView);
        }
    }
    QueryPerformanceCounter(&end);

    CloseHandle(hMappingB);
    CloseHandle(hFileB);

    double timeModeB = static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    std::wcout << L"   -> Время выполнения Режима Б: " << timeModeB << L" мс" << std::endl;

    std::wcout << L"\n--- АНАЛИЗ НАКЛАДНЫХ РАСХОДОВ TLB FLUSH ---" << std::endl;
    std::wcout << L"1. При частых вызовах UnmapViewOfFile ядро ОС Windows вынуждено:" << std::endl;
    std::wcout << L"   - Изменять таблицы страниц (Page Tables) процесса;" << std::endl;
    std::wcout << L"   - Выполнять аппаратную инструкцию сброса буфера ассоциативной трансляции (TLB Flush / INVLPG);" << std::endl;
    std::wcout << L"   - Обрабатывать Page Fault при следующем MapViewOfFile." << std::endl;
    std::wcout << L"2. В Режиме А системный файловый кэш держит страницы непрерывно в ОЗУ без переключения маппингов." << std::endl;

    return 0;
}

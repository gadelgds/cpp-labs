#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <random>

const DWORD FILE_SIZE_10MB = 10 * 1024 * 1024; // 10 МБ
const DWORD SECTOR_SIZE = 4096;                // 4 КБ (размер сектора диска)

// Создание тестового файла на 10 МБ со случайными символами
bool GenerateRandomFile(const wchar_t* path, DWORD sizeBytes) {
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    const DWORD chunkSize = 64 * 1024;
    std::vector<char> buffer(chunkSize);
    std::mt19937 rng(42);
    for (size_t i = 0; i < chunkSize; ++i) buffer[i] = static_cast<char>('A' + (rng() % 26));

    DWORD writtenTotal = 0;
    while (writtenTotal < sizeBytes) {
        DWORD toWrite = min(chunkSize, sizeBytes - writtenTotal);
        DWORD written = 0;
        WriteFile(hFile, buffer.data(), toWrite, &written, NULL);
        writtenTotal += written;
    }
    CloseHandle(hFile);
    return true;
}

// Замер времени чтения через QueryPerformanceCounter
double BenchmarkReading(const wchar_t* path, DWORD flags, void* buffer, DWORD readChunkSize) {
    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, flags, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return -1.0;

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    DWORD bytesRead = 0;
    DWORD totalRead = 0;
    while (totalRead < FILE_SIZE_10MB) {
        if (!ReadFile(hFile, buffer, readChunkSize, &bytesRead, NULL) || bytesRead == 0) {
            break;
        }
        totalRead += bytesRead;
    }

    QueryPerformanceCounter(&end);
    CloseHandle(hFile);

    return static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
}

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    _setmode(_fileno(stdout), _O_U8TEXT);
    #endif

    const wchar_t* testFile = L"test_direct_io_10mb.dat";
    std::wcout << L"=== ПРАКТИКА №1: НЕБУФЕРИЗИРОВАННЫЙ ВВОД-ВЫВОД (NO_BUFFERING) ===" << std::endl;

    std::wcout << L"1. Создание тестового файла 10 МБ со случайными данными..." << std::endl;
    GenerateRandomFile(testFile, FILE_SIZE_10MB);

    // 2. Демонстрация ошибки ERROR_INVALID_PARAMETER (код 87)
    std::wcout << L"\n2. Попытка чтения в невыровненный буфер (char unalignedBuf[1000])..." << std::endl;
    HANDLE hDirect = CreateFileW(testFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
                                 FILE_FLAG_NO_BUFFERING, NULL);

    if (hDirect != INVALID_HANDLE_VALUE) {
        char unalignedBuf[1000]; // Не выровнен по границе 4096 байт, размер не кратен 4096
        DWORD bytesRead = 0;
        BOOL result = ReadFile(hDirect, unalignedBuf, sizeof(unalignedBuf), &bytesRead, NULL);
        if (!result) {
            DWORD err = GetLastError();
            std::wcout << L"   [ОШИБКА ПОЛУЧЕНА]: ReadFile вернул FALSE, GetLastError() = " << err << std::endl;
            if (err == ERROR_INVALID_PARAMETER) {
                std::wcout << L"   -> Код 87 (ERROR_INVALID_PARAMETER): буфер не выровнен по границе сектора (4096 байт)!" << std::endl;
            }
        }
        CloseHandle(hDirect);
    }

    // 3. Исправление ошибки: выделение памяти через VirtualAlloc (выравнивание по 4096 байт)
    std::wcout << L"\n3. Выделение выровненного буфера через VirtualAlloc (64 КБ)..." << std::endl;
    const DWORD alignedChunkSize = 64 * 1024; // 64 КБ (кратно 4096)
    void* alignedBuffer = VirtualAlloc(NULL, alignedChunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!alignedBuffer) {
        std::wcout << L"Ошибка VirtualAlloc!" << std::endl;
        return 1;
    }
    std::wcout << L"   Адрес выровненного буфера: " << alignedBuffer 
               << L" (Адрес % 4096 == " << ((UINT_PTR)alignedBuffer % SECTOR_SIZE) << L")" << std::endl;

    // 4. Сравнение скорости чтения с кэшированием и без кэширования
    std::wcout << L"\n4. Сравнение производительности чтения 10 МБ (QueryPerformanceCounter):" << std::endl;

    // Тест А: Со стандартным кэшированием ОС (FILE_ATTRIBUTE_NORMAL)
    double timeCached = BenchmarkReading(testFile, FILE_ATTRIBUTE_NORMAL, alignedBuffer, alignedChunkSize);
    std::wcout << L"   А) Стандартное чтение (с системным кэшем):  " << timeCached << L" мс" << std::endl;

    // Тест Б: Небуферизированный ввод-вывод (FILE_FLAG_NO_BUFFERING)
    double timeDirect = BenchmarkReading(testFile, FILE_FLAG_NO_BUFFERING, alignedBuffer, alignedChunkSize);
    std::wcout << L"   Б) Прямое чтение (без промежуточного кэша): " << timeDirect << L" мс" << std::endl;

    // Освобождение памяти
    VirtualFree(alignedBuffer, 0, MEM_RELEASE);

    return 0;
}

#include <windows.h>
#include <iostream>
#include <iomanip>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    // 1. Создание/открытие файла (объект ядра File)
    HANDLE hFile = CreateFileW(
        L"temp_kernel_demo.tmp",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
        NULL
    );

    // 2. Создание события (объект ядра Event)
    HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    // 3. Получение дескриптора текущего процесса (псевдо-хэндл)
    HANDLE hProcess = GetCurrentProcess();

    std::cout << "=== Объекты ядра Windows и их дескрипторы ===" << std::endl;
    std::cout << "1. File Handle (CreateFile):          " << (void*)hFile    << " (UINT_PTR: " << (UINT_PTR)hFile    << ")" << std::endl;
    std::cout << "2. Event Handle (CreateEvent):        " << (void*)hEvent   << " (UINT_PTR: " << (UINT_PTR)hEvent   << ")" << std::endl;
    std::cout << "3. Current Process (GetCurrentProcess): " << (void*)hProcess << " (UINT_PTR: " << (UINT_PTR)hProcess << ")" << std::endl;

    // Закрытие реальных дескрипторов
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
    if (hEvent != NULL) {
        CloseHandle(hEvent);
    }
    // Псевдо-дескриптор hProcess закрывать через CloseHandle не нужно!

    return 0;
}

#include <windows.h>
#include <iostream>
#include <iomanip>

const DWORD CHUNK_SIZE = 64 * 1024; // 64 КБ на один блок передачи

// Функция создания тестового файла заданного размера (в мегабайтах)
bool CreateDummyFile(const wchar_t* path, DWORD sizeMB) {
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER liSize;
    liSize.QuadPart = (LONGLONG)sizeMB * 1024 * 1024;
    
    // Быстрое задание размера файла через SetFilePointerEx и SetEndOfFile
    if (!SetFilePointerEx(hFile, liSize, NULL, FILE_BEGIN) || !SetEndOfFile(hFile)) {
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    return true;
}

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    const wchar_t* srcPath = L"source_large.dat";
    const wchar_t* dstPath = L"destination_copy.dat";

    std::wcout << L"=== АСИНХРОННОЕ КОПИРОВАНИЕ ФАЙЛА (OVERLAPPED + hEvent) ===" << std::endl;

    // Проверяем наличие исходного файла. Если нет — создаем демонстрационный файл 1024 МБ (1 ГБ)
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(srcPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        std::wcout << L"Создание тестового файла на 1024 МБ (1 ГБ)..." << std::endl;
        if (!CreateDummyFile(srcPath, 1024)) {
            std::wcout << L"Ошибка при создании тестового файла: " << GetLastError() << std::endl;
            return 1;
        }
        std::wcout << L"Тестовый файл успешно создан." << std::endl;
    } else {
        FindClose(hFind);
    }

    // 1. Открытие исходного файла с флагом FILE_FLAG_OVERLAPPED
    HANDLE hSrc = CreateFileW(
        srcPath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );
    if (hSrc == INVALID_HANDLE_VALUE) {
        std::wcout << L"Не удалось открыть исходный файл: " << GetLastError() << std::endl;
        return 1;
    }

    // 2. Создание целевого файла с флагом FILE_FLAG_OVERLAPPED
    HANDLE hDst = CreateFileW(
        dstPath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_FLAG_OVERLAPPED,
        NULL
    );
    if (hDst == INVALID_HANDLE_VALUE) {
        std::wcout << L"Не удалось создать целевой файл: " << GetLastError() << std::endl;
        CloseHandle(hSrc);
        return 1;
    }

    // Получаем полный размер файла
    LARGE_INTEGER fileSize;
    GetFileSizeEx(hSrc, &fileSize);
    std::wcout << L"Размер исходного файла: " << (fileSize.QuadPart / (1024 * 1024)) << L" МБ" << std::endl;

    // 3. Создание событий для контроля завершения операций чтения и записи
    HANDLE hReadEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    HANDLE hWriteEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    char* buffer = new char[CHUNK_SIZE];

    OVERLAPPED ovRead;
    OVERLAPPED ovWrite;
    memset(&ovRead, 0, sizeof(ovRead));
    memset(&ovWrite, 0, sizeof(ovWrite));

    ovRead.hEvent = hReadEvent;
    ovWrite.hEvent = hWriteEvent;

    ULONGLONG currentOffset = 0;
    DWORD bytesTransferred = 0;

    std::wcout << L"Запуск асинхронного копирования блоками по " << (CHUNK_SIZE / 1024) << L" КБ..." << std::endl;

    while (currentOffset < (ULONGLONG)fileSize.QuadPart) {
        DWORD bytesToRead = CHUNK_SIZE;
        if (currentOffset + bytesToRead > (ULONGLONG)fileSize.QuadPart) {
            bytesToRead = (DWORD)(fileSize.QuadPart - currentOffset);
        }

        // Задаем смещение для чтения
        ovRead.Offset = (DWORD)(currentOffset & 0xFFFFFFFF);
        ovRead.OffsetHigh = (DWORD)(currentOffset >> 32);
        ResetEvent(hReadEvent);

        // Асинхронный вызов ReadFile
        BOOL readRet = ReadFile(hSrc, buffer, bytesToRead, NULL, &ovRead);
        if (!readRet && GetLastError() != ERROR_IO_PENDING) {
            std::wcout << L"Ошибка при асинхронном чтении: " << GetLastError() << std::endl;
            break;
        }

        // Ожидаем завершения чтения через событие hEvent
        WaitForSingleObject(hReadEvent, INFINITE);
        GetOverlappedResult(hSrc, &ovRead, &bytesTransferred, FALSE);

        if (bytesTransferred == 0) break;

        // Задаем смещение для записи
        ovWrite.Offset = ovRead.Offset;
        ovWrite.OffsetHigh = ovRead.OffsetHigh;
        ResetEvent(hWriteEvent);

        // Асинхронный вызов WriteFile
        BOOL writeRet = WriteFile(hDst, buffer, bytesTransferred, NULL, &ovWrite);
        if (!writeRet && GetLastError() != ERROR_IO_PENDING) {
            std::wcout << L"Ошибка при асинхронной записи: " << GetLastError() << std::endl;
            break;
        }

        // Ожидаем завершения записи через событие hEvent
        WaitForSingleObject(hWriteEvent, INFINITE);
        GetOverlappedResult(hDst, &ovWrite, &bytesTransferred, FALSE);

        currentOffset += bytesTransferred;

        // Вывод прогресса в консоль
        double percent = ((double)currentOffset / fileSize.QuadPart) * 100.0;
        std::wcout << L"\rПрогресс копирования: " << std::fixed << std::setprecision(1) << percent << L"% ("
                   << (currentOffset / (1024 * 1024)) << L" / " << (fileSize.QuadPart / (1024 * 1024)) << L" МБ)" << std::flush;
    }

    std::wcout << L"\nКопирование успешно завершено!" << std::endl;

    // Освобождение ресурсов и закрытие дескрипторов
    CloseHandle(hReadEvent);
    CloseHandle(hWriteEvent);
    CloseHandle(hSrc);
    CloseHandle(hDst);
    delete[] buffer;

    return 0;
}

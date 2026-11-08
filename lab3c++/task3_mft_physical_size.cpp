#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

void investigateFile(const wchar_t* fileName, DWORD bytesToWrite) {
    HANDLE hFile = CreateFileW(
        fileName,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::wcout << L"Ошибка создания файла: " << GetLastError() << std::endl;
        return;
    }

    char* buffer = new char[bytesToWrite];
    memset(buffer, 'A', bytesToWrite);

    DWORD bytesWritten = 0;
    WriteFile(hFile, buffer, bytesToWrite, &bytesWritten, NULL);

    CloseHandle(hFile);
    delete[] buffer;

    DWORD highSize = 0;
    DWORD physicalSize = GetCompressedFileSizeW(fileName, &highSize);

    std::wcout << L"Файл: " << fileName << std::endl;
    std::wcout << L"  Записано данных (логический размер): " << bytesToWrite << L" байт." << std::endl;
    std::wcout << L"  Занято на диске (физический размер): " << physicalSize << L" байт." << std::endl;
    std::wcout << L"--------------------------------------------------" << std::endl;
}

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    _setmode(_fileno(stdout), _O_U8TEXT);
    #endif

    std::wcout << L"=== ИССЛЕДОВАНИЕ ФИЗИЧЕСКОГО РАЗМЕРА ФАЙЛА ВНЕ ТАБЛИЦЫ MFT ===" << std::endl;

    investigateFile(L"small_file.txt", 200);   // Кейс 1: 200 байт (резидентный в MFT)
    investigateFile(L"large_file.txt", 2000); // Кейс 2: 2000 байт (нерезидентный)

    return 0;
}

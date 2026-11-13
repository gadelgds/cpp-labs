#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

void investigateFile(const wchar_t* fileName, DWORD bytesToWrite) {
    HANDLE hFile = CreateFileW(
        fileName,
        GENERIC_READ | GENERIC_WRITE,
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
    delete[] buffer;

    // ПРЯМОЙ ПУТЬ: спрашиваем у самой NTFS, сколько места выделено под файл.
    // FILE_STANDARD_INFO.AllocationSize — это реальный выделенный размер:
    // 0 для резидентного файла (данные лежат внутри записи MFT, кластеры не
    // выделялись), для нерезидентного — число кластеров × размер кластера.
    // Запрос делается по открытому хэндлу, до CloseHandle.
    FILE_STANDARD_INFO stdInfo = {};
    LONGLONG physicalSize = -1;
    if (GetFileInformationByHandleEx(hFile, FileStandardInfo, &stdInfo, sizeof(stdInfo))) {
        physicalSize = stdInfo.AllocationSize.QuadPart;
    } else {
        std::wcout << L"Ошибка GetFileInformationByHandleEx: " << GetLastError() << std::endl;
    }

    CloseHandle(hFile);

    // ДЛЯ СРАВНЕНИЯ: ловушка GetCompressedFileSizeW.
    // Несмотря на имя, для обычного (несжатого, не разреженного) файла она
    // возвращает ЛОГИЧЕСКИЙ размер, как GetFileSize. Так написано в самой
    // документации Microsoft: "If the file is not compressed or a sparse
    // file, the value obtained is the actual file size, the same as the value
    // returned by a call to GetFileSize". Реальный размер на диске она даёт
    // только для сжатых и разреженных файлов. Поэтому старый вариант этой
    // программы печатал 200 и 2000 вместо правильных 0 и 4096.
    DWORD highSize = 0;
    DWORD compressedSize = GetCompressedFileSizeW(fileName, &highSize);
    if (compressedSize == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
        std::wcout << L"Ошибка GetCompressedFileSizeW: " << GetLastError() << std::endl;
    }

    std::wcout << L"Файл: " << fileName << std::endl;
    std::wcout << L"  Записано данных (логический размер): " << bytesToWrite << L" байт." << std::endl;
    std::wcout << L"  Занято на диске (физический размер, напрямую): " << physicalSize << L" байт." << std::endl;
    std::wcout << L"  GetCompressedFileSizeW (для сравнения): " << compressedSize << L" байт." << std::endl;
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

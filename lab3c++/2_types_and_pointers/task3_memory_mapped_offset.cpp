#include <windows.h>
#include <iostream>
#include <string>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== САМОСТОЯТЕЛЬНО 2: ПРОЕЦИРОВАНИЕ ФАЙЛА С ФИКСИРОВАННЫМ СМЕЩЕНИЕМ ===" << std::endl;

    const wchar_t* filename = L"mapped_large_test.txt";

    // 1. Получаем информацию об архитектуре и гранулярности распределения памяти Windows
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD allocGranularity = sysInfo.dwAllocationGranularity; // Обычно 65536 байт (64 КБ)

    std::cout << "Системная гранулярность выделения памяти (Allocation Granularity): "
              << allocGranularity << " байт (" << (allocGranularity / 1024) << " КБ)" << std::endl;

    // 2. Создаем тестовый файл достаточного размера (например, 256 КБ = 4 * 64 КБ)
    HANDLE hFile = CreateFileW(
        filename,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cout << "Ошибка создания файла: " << GetLastError() << std::endl;
        return 1;
    }

    // Заполняем файл данными: 4 блока по 64 КБ с уникальными маркерами
    const DWORD totalFileSize = 4 * allocGranularity; // 256 КБ
    std::string fillData(totalFileSize, '.');

    // Размещаем маркер во 2-м блоке (смещение 2 * allocGranularity = 128 КБ)
    std::string secretMessage = "[СЕКРЕТНЫЙ ТЕКСТ НА СМЕЩЕНИИ 128 КБ]";
    size_t targetOffset = 2 * allocGranularity;
    memcpy(&fillData[targetOffset], secretMessage.c_str(), secretMessage.length());

    DWORD bytesWritten = 0;
    WriteFile(hFile, fillData.data(), totalFileSize, &bytesWritten, NULL);

    // 3. Создаем объект проекции файла (File Mapping Object)
    HANDLE hMapping = CreateFileMappingW(
        hFile,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
    );

    if (hMapping == NULL) {
        std::cout << "Ошибка CreateFileMapping: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return 1;
    }

    // 4. Смещение для MapViewOfFile ОБЯЗАНО быть строго кратным allocGranularity!
    // Демонстрируем расчет смещения:
    DWORD offsetLow = (DWORD)(targetOffset & 0xFFFFFFFF);
    DWORD offsetHigh = (DWORD)((targetOffset >> 32) & 0xFFFFFFFF);
    SIZE_T viewSize = 1024; // Отображаем окно размером 1024 байта

    std::cout << "\nЗапрос проецирования участка файла:" << std::endl;
    std::cout << "  Смещение: " << targetOffset << " байт (кратно " << allocGranularity << ")" << std::endl;
    std::cout << "  Размер проецируемого окна: " << viewSize << " байт" << std::endl;

    // 5. Проецируем срез файла в виртуальную память процесса
    const char* pView = static_cast<const char*>(MapViewOfFile(
        hMapping,
        FILE_MAP_READ,
        offsetHigh,
        offsetLow,
        viewSize
    ));

    if (pView == NULL) {
        std::cout << "Ошибка MapViewOfFile: " << GetLastError() << std::endl;
    } else {
        std::cout << "  Успешно спроецировано по виртуальному адресу: " << (void*)pView << std::endl;

        // Читаем данные из памяти спроецированного окна
        std::string readChunk(pView, secretMessage.length());
        std::cout << "  Прочитанные данные: \"" << readChunk << "\"" << std::endl;

        // Отмонтируем проекцию
        UnmapViewOfFile(pView);
        std::cout << "Окно проекции успешно размонтировано (UnmapViewOfFile)." << std::endl;
    }

    // 6. Закрытие дескрипторов
    CloseHandle(hMapping);
    CloseHandle(hFile);

    return 0;
}

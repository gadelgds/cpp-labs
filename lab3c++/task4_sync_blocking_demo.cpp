#include <windows.h>
#include <iostream>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::wcout << L"=== ТЕСТ СИНХРОННОЙ БЛОКИРОВКИ ПОТОКА ПРИ ВВОДЕ-ВЫВОДЕ ===" << std::endl;
    std::wcout << L"Для начала эксперимента нажмите [Enter]...";
    std::cin.get();

    const wchar_t* filePath = L"heavy_test_file.tmp";

    // Проверяем наличие файла. Если нет — создаем файл размером 256 МБ
    HANDLE hCheck = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hCheck == INVALID_HANDLE_VALUE) {
        std::wcout << L"Создание демонстрационного файла 256 МБ для эмуляции нагрузки..." << std::endl;
        HANDLE hNew = CreateFileW(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hNew != INVALID_HANDLE_VALUE) {
            LARGE_INTEGER sz;
            sz.QuadPart = 256LL * 1024 * 1024;
            SetFilePointerEx(hNew, sz, NULL, FILE_BEGIN);
            SetEndOfFile(hNew);
            CloseHandle(hNew);
        }
    } else {
        CloseHandle(hCheck);
    }

    HANDLE hFile = CreateFileW(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::wcout << L"Ошибка открытия файла! Код: " << GetLastError() << std::endl;
        return 1;
    }

    DWORD bufferSize = 64 * 1024 * 1024; // Буфер 64 МБ
    char* buffer = new char[bufferSize];
    DWORD bytesRead = 0;

    std::wcout << L"Начинаем процесс: поток эмулирует работу интерфейса (выводит тики таймера)..." << std::endl;
    Sleep(1000);

    for (int i = 1; i <= 10; i++) {
        std::wcout << L"Интерфейс: Тик " << i << L" [Поток активен, State: Running]" << std::endl;
        Sleep(500); // Имитация обработки событий пользовательского интерфейса

        if (i == 5) {
            std::wcout << L"\n!!! ВНИМАНИЕ: Вызываем синхронный ReadFile() на 64 МБ!" << std::endl;
            std::wcout << L"!!! Поток переходит в ядре в состояние Waiting (заблокирован драйвером)." << std::endl;
            std::wcout << L"!!! В этот момент GUI-окно программы зависает ('Не отвечает')...\n" << std::endl;

            // Синхронный вызов. Текущий поток приостанавливается планировщиком Windows
            ReadFile(hFile, buffer, bufferSize, &bytesRead, NULL);

            std::wcout << L"!!! ReadFile() завершился! Прочитано " << (bytesRead / (1024 * 1024))
                       << L" МБ. Поток разблокирован и вернулся к выполнению.\n" << std::endl;
        }
    }

    std::wcout << L"Эксперимент окончен." << std::endl;

    CloseHandle(hFile);
    delete[] buffer;

    return 0;
}

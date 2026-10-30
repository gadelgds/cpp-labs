#include <windows.h>
#include <iostream>
#include <vector>

// Функция анализа размещения файла в NTFS (резидентный в MFT vs нерезидентный с экстентами)
void AnalyzeNtfsFileAllocation(const wchar_t* filePath, DWORD sizeInBytes) {
    // Создаем тестовый файл нужного размера
    HANDLE hFile = CreateFileW(
        filePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::wcout << L"Ошибка открытия файла: " << GetLastError() << std::endl;
        return;
    }

    std::vector<char> buffer(sizeInBytes, 'X');
    DWORD written = 0;
    WriteFile(hFile, buffer.data(), sizeInBytes, &written, NULL);

    // Запрос цепочек кластеров (Data Runs / Extents) через FSCTL_GET_RETRIEVAL_POINTERS
    STARTING_VCN_INPUT_BUFFER inputVcn;
    inputVcn.StartingVcn.QuadPart = 0;

    BYTE outBuf[1024];
    DWORD bytesReturned = 0;

    BOOL fsctlSuccess = DeviceIoControl(
        hFile,
        FSCTL_GET_RETRIEVAL_POINTERS,
        &inputVcn,
        sizeof(inputVcn),
        outBuf,
        sizeof(outBuf),
        &bytesReturned,
        NULL
    );

    std::wcout << L"--------------------------------------------------" << std::endl;
    std::wcout << L"Файл: " << filePath << L" (Размер: " << sizeInBytes << L" байт)" << std::endl;

    if (!fsctlSuccess) {
        DWORD err = GetLastError();
        if (err == ERROR_HANDLE_EOF) {
            std::wcout << L"Статус MFT: РЕЗИДЕНТНЫЙ ФАЙЛ (Resident Attribute $DATA)" << std::endl;
            std::wcout << L"  -> Данные файла целиком хранятся внутри 1024-байтной записи MFT!" << std::endl;
            std::wcout << L"  -> Файл не занимает отдельных кластеров на дисковом томе (Extents = 0)." << std::endl;
        } else {
            std::wcout << L"Ошибка FSCTL_GET_RETRIEVAL_POINTERS: " << err << std::endl;
        }
    } else {
        RETRIEVAL_POINTERS_BUFFER* rp = reinterpret_cast<RETRIEVAL_POINTERS_BUFFER*>(outBuf);
        std::wcout << L"Статус MFT: НЕРЕЗИДЕНТНЫЙ ФАЙЛ (Non-Resident Attribute $DATA)" << std::endl;
        std::wcout << L"  -> Данные не поместились в MFT-запись и вынесены в отдельные кластеры тома." << std::endl;
        std::wcout << L"  -> Количество цепочек кластеров (Extents): " << rp->ExtentCount << std::endl;
        for (DWORD i = 0; i < rp->ExtentCount; ++i) {
            std::wcout << L"     Цепочка " << i << L": LCN = " << rp->Extents[i].Lcn.QuadPart 
                       << L", NextVCN = " << rp->Extents[i].NextVcn.QuadPart << std::endl;
        }
    }

    CloseHandle(hFile);
}

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::wcout << L"=== АНАЛИЗ ЧТЕНИЯ MFT: РЕЗИДЕНТНЫЕ И НЕРЕЗИДЕНТНЫЕ ДАННЫЕ ===" << std::endl;

    // Сравнение 1: Маленький файл (200 байт — строго помещается в MFT)
    AnalyzeNtfsFileAllocation(L"test_resident_200b.dat", 200);

    // Сравнение 2: Большой файл (2500 байт — не помещается в 1 КБ запись MFT)
    AnalyzeNtfsFileAllocation(L"test_nonresident_2500b.dat", 2500);

    // Сравнение 3: Файл объемом 500 КБ (512 000 байт — нерезидентный массив данных)
    AnalyzeNtfsFileAllocation(L"test_large_500kb.dat", 500 * 1024);

    return 0;
}

#include <windows.h>
#include <iostream>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ПРАКТИКА №3: ВЫДЕЛЕНИЕ ПАМЯТИ НАПРЯМУЮ У ЯДРА WINDOWS (VirtualAlloc) ===" << std::endl;

    // 1. Запрос размера массива у пользователя
    size_t count = 10000;
    std::cout << "Введите размер массива целых чисел (по умолчанию 10000): ";
    if (!(std::cin >> count) || count == 0) {
        count = 10000;
    }

    size_t sizeInBytes = count * sizeof(int);
    std::cout << "Запрошено элементов: " << count 
              << " (" << sizeInBytes << " байт / ~" << (sizeInBytes / 1024) << " КБ)" << std::endl;

    // 2. Выделение страниц виртуальной памяти напрямую у ядра Windows через VirtualAlloc
    int* pArray = static_cast<int*>(VirtualAlloc(
        NULL,                   // Ядро само выбирает базовый адрес в адресном пространстве
        sizeInBytes,            // Объем выделяемой памяти в байтах
        MEM_COMMIT | MEM_RESERVE, // Резервирование адресного диапазона и выделение физических страниц
        PAGE_READWRITE          // Права доступа: чтение и запись
    ));

    if (pArray == NULL) {
        std::cout << "Ошибка выделения памяти у ядра: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Память успешно выделена ядром по адресу: " << static_cast<void*>(pArray) << std::endl;

    // 3. Заполнение массива квадратами индексов: arr[i] = i * i
    for (size_t i = 0; i < count; ++i) {
        pArray[i] = static_cast<int>(i * i);
    }

    // 4. Вывод первых 10 элементов массива на экран
    std::cout << "\nПервые 10 элементов массива (квадраты индексов):" << std::endl;
    for (size_t i = 0; i < 10 && i < count; ++i) {
        std::cout << "  pArray[" << i << "] = " << pArray[i] << std::endl;
    }

    // 5. Освобождение памяти напрямую ядру ОС через VirtualFree
    BOOL freeSuccess = VirtualFree(
        pArray,         // Базовый адрес освобождаемого блока
        0,              // Для MEM_RELEASE размер обязан быть равен 0
        MEM_RELEASE     // Полное освобождение физических страниц и де-резервирование виртуального диапазона
    );

    if (freeSuccess) {
        std::cout << "\nПамять успешно возвращена ядру Windows (VirtualFree MEM_RELEASE)." << std::endl;
    } else {
        std::cout << "\nОшибка освобождения памяти: " << GetLastError() << std::endl;
    }

    return 0;
}

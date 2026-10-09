/**
 * @file task6_num_to_string.cpp
 * @brief Задача 6 (Фото 2, Задача 4): Преобразование числа в массив и конкатенация.
 * 
 * УСЛОВИЕ:
 * Есть строка "Клиент №". Дано целое число int ID = 245;.
 * Переведите это число в символьный массив с помощью функции sprintf (или itoa)
 * и добавьте его в конец исходной строки, чтобы получилось "Клиент №245".
 * 
 * ПОШАГОВОЕ ОБЪЯСНЕНИЕ РЕШЕНИЯ:
 * 1. Выделение буфера достаточного размера:
 *    - Исходная строка "Клиент №" в кодировке UTF-8 занимает 15 байт + 1 байт '\0'.
 *    - Чтобы к ней можно было безопасно приписать символы числа без переполнения буфера
 *      (Buffer Overflow), объявляем массив с запасом: char clientStr[128] = "Клиент №";
 * 
 * 2. Преобразование целого числа в символьный массив:
 *    - Вариант A (sprintf / snprintf):
 *      char idBuffer[32];
 *      snprintf(idBuffer, sizeof(idBuffer), "%d", ID);
 *      Функция форматирует число ID в десятичном виде и записывает ASCII-символы в массив.
 *    - Вариант B (itoa / _itoa_s):
 *      itoa(ID, idBuffer, 10);
 *      Функция преобразует целое число в строку в заданной системе счисления (10).
 * 
 * 3. Конкатенация строк (склеивание):
 *    - strcat(clientStr, idBuffer);
 *      Функция находит завершающий '\0' в clientStr и дописывает туда символы из idBuffer,
 *      добавляя в конце новый '\0'.
 * 
 * 4. Итоговая строка: "Клиент №245".
 */

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ЗАДАЧА 6: Преобразование числа в массив и конкатенация ===" << std::endl;

    // Шаг 1: Исходные данные
    int ID = 245;

    // ВАРИАНТ 1: Использование sprintf и strcat (классический стандарт Си)
    std::cout << "\n--- Способ 1: Использование sprintf() и strcat() ---" << std::endl;
    char clientStr1[128] = "Клиент №";
    char idStr1[32];

    std::cout << "[Шаг 1.1] Исходная строка: \"" << clientStr1 << "\"" << std::endl;
    std::cout << "[Шаг 1.2] Исходное число ID: " << ID << std::endl;

    // Преобразуем число в символьный массив
    sprintf(idStr1, "%d", ID);
    std::cout << "[Шаг 1.3] Число переведено в массив idStr1: \"" << idStr1 << "\"" << std::endl;

    // Добавляем в конец исходной строки
    strcat(clientStr1, idStr1);
    std::cout << "[Шаг 1.4] Итоговая строка после strcat:   \"" << clientStr1 << "\"" << std::endl;


    // ВАРИАНТ 2: Использование функции itoa() / _itoa()
    std::cout << "\n--- Способ 2: Использование itoa() ---" << std::endl;
    char clientStr2[128] = "Клиент №";
    char idStr2[32];

    #if defined(_MSC_VER)
    _itoa(ID, idStr2, 10); // Безопасный аналог для MSVC компилятора
    #else
    sprintf(idStr2, "%d", ID); // Fallback для строгого ISO C++
    #endif

    std::cout << "[Шаг 2.1] itoa преобразует ID в строку по основанию 10: \"" << idStr2 << "\"" << std::endl;
    strcat(clientStr2, idStr2);
    std::cout << "[Шаг 2.2] Результат: \"" << clientStr2 << "\"" << std::endl;


    // ВАРИАНТ 3: Прямая запись через sprintf без промежуточного буфера
    std::cout << "\n--- Способ 3: Прямая дозапись в конец строки через sprintf ---" << std::endl;
    char clientStr3[128] = "Клиент №";
    // Смещение на текущую длину строки strlen:
    sprintf(clientStr3 + strlen(clientStr3), "%d", ID);
    std::cout << "[Результат] \"" << clientStr3 << "\"" << std::endl;

    return 0;
}

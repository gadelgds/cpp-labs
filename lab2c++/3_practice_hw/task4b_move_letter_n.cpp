#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <cstring>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    // Исходная переменная объявлена const — её НИКТО не меняет
    const char clientStr[] = "Клиент";
    int ID = 245;

    // Число -> строка
    char idStr[32];
    sprintf(idStr, "%d", ID);

    // Буква 'н' (в UTF-8 это два байта)
    const char letterN[] = "н";

    // НОВАЯ строка-результат, исходную переменную не трогаем
    char result[128] = "";

    // 1. Кладём букву 'н' в НАЧАЛО результата
    strcat(result, letterN);

    // 2. Ищем букву 'н' в исходном слове
    const char* posN = strstr(clientStr, letterN);
    if (posN != nullptr) {
        int before = (int)(posN - clientStr);      // байт до буквы 'н'

        // 3. Копируем начало слова ("Клие") — уже без буквы 'н'
        strncat(result, clientStr, before);

        // 4. Копируем остаток после буквы 'н' ("т")
        strcat(result, posN + strlen(letterN));
    }

    // 5. Приклеиваем число
    strcat(result, idStr);

    std::cout << "Исходная строка (не изменена): " << clientStr << std::endl;
    std::cout << "Результат:                     " << result << std::endl;

    return 0;
}

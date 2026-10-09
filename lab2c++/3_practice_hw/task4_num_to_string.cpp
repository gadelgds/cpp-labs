#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <cstring>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    // Исходная строка с запасом памяти для добавления числа
    char clientStr[128] = "Клиент №";
    int ID = 245;

    // Символьный массив для записи числа
    char idStr[32];

    // Переводим число ID в символьный массив с помощью sprintf
    sprintf(idStr, "%d", ID);

    // Добавляем символьный массив в конец исходной строки
    strcat(clientStr, idStr);

    // Выводим полученную строку: "Клиент №245"
    std::cout << clientStr << std::endl;

    return 0;
}

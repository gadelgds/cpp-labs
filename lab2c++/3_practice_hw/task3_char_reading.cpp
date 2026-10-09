#include <iostream>
#include <cstdio>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "Введите строку текста и нажмите [Enter]: ";

    int ch;
    int spaceCount = 0;

    // Считываем символы по одному до переноса строки '\n' или конца потока EOF
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch == ' ') {
            spaceCount++;
        }
    }

    std::cout << "Количество введенных пробелов: " << spaceCount << std::endl;

    return 0;
}

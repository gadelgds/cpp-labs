#include <iostream>
#include <iomanip>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    int price = 350;        // Цена товара в рублях
    int money = 500;        // Купюра покупателя
    int change = money - price; // Сдача

    double discount = 0.15; // Скидка 15%
    double discountedPrice = price * (1.0 - discount); // Цена со скидкой

    std::cout << "Цена товара:          " << price << " руб." << std::endl;
    std::cout << "Купюра покупателя:    " << money << " руб." << std::endl;
    std::cout << "Сдача:                " << change << " руб." << std::endl;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Скидка:               " << (discount * 100.0) << "%" << std::endl;
    std::cout << "Цена со скидкой:      " << discountedPrice << " руб." << std::endl;

    return 0;
}

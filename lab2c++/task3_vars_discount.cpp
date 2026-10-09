/**
 * @file task3_vars_discount.cpp
 * @brief Задача 3 (Фото 2, Задача 1): Переменные (целые и действительные числа).
 * 
 * УСЛОВИЕ:
 * Даны две переменные: price (цена товара, целое число в рублях) и money 
 * (купюра, которую дал покупатель, целое число). Посчитайте сдачу.
 * Дополнительно объявите переменную discount (скидка, действительное число, 
 * например 0.15 для 15%) и посчитайте цену со скидкой.
 * 
 * ПОШАГОВОЕ ОБЪЯСНЕНИЕ РЕШЕНИЯ:
 * 1. Выбор типов данных:
 *    - price (цена) — тип 'int' (целое число рублей по условию);
 *    - money (купюра) — тип 'int' (целое число рублей по условию);
 *    - change (сдача) — тип 'int' (целое число рублей: money - price);
 *    - discount (скидка) — тип 'double' (действительное число, напр. 0.15);
 *    - discountedPrice (цена со скидкой) — тип 'double', так как умножение
 *      на дробный коэффициент дает вещественный результат.
 * 
 * 2. Расчет сдачи:
 *    change = money - price;
 *    Если money >= price, покупатель получает сдачу.
 *    Если money < price, средств недостаточно.
 * 
 * 3. Расчет цены со скидкой:
 *    discountedPrice = price * (1.0 - discount);
 *    Или: discountedPrice = price - (price * discount);
 *    Например, при цене 500 руб. и скидке 0.15 (15%):
 *    500 * (1.0 - 0.15) = 500 * 0.85 = 425.0 руб.
 */

#include <iostream>
#include <iomanip>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ЗАДАЧА 3: Переменные, сдача и расчет скидки ===" << std::endl;

    // Шаг 1: Объявление и инициализация базовых переменных
    int price = 350;     // Цена товара в рублях (целое число)
    int money = 500;     // Купюра покупателя (целое число)

    // Шаг 2: Расчет сдачи
    int change = money - price;

    // Шаг 3: Объявление скидки и расчет цены со скидкой
    double discount = 0.15; // Скидка 15% (действительное число)
    double discountedPrice = price * (1.0 - discount);
    double discountSum = price * discount; // Размер экономии в рублях

    // Вывод результатов
    std::cout << "\n[Исходные данные]" << std::endl;
    std::cout << "  Цена товара (price):          " << price << " руб." << std::endl;
    std::cout << "  Купюра покупателя (money):     " << money << " руб." << std::endl;
    std::cout << "  Скидка (discount):             " << (discount * 100) << "% (" << discount << ")" << std::endl;

    std::cout << "\n[Расчеты]" << std::endl;
    std::cout << "  1. Сдача (money - price):      " << change << " руб." << std::endl;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  2. Сумма скидки:               " << discountSum << " руб." << std::endl;
    std::cout << "  3. Цена со скидкой:            " << discountedPrice << " руб." << std::endl;

    // Расчет новой сдачи, если применяется скидка
    double changeWithDiscount = money - discountedPrice;
    std::cout << "  4. Сдача с учетом скидки:      " << changeWithDiscount << " руб." << std::endl;

    return 0;
}

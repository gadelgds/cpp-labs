/**
 * @file task4_type_casting.cpp
 * @brief Задача 4 (Фото 2, Задача 2): Преобразование типов (явное и неявное).
 * 
 * УСЛОВИЕ:
 * Даны два целых числа: int a = 7; int b = 3;. Вычислите a / b.
 * Второй вариант: модифицируйте код с помощью явного приведения типов (double).
 * Округлите результат до 3 знаков после запятой.
 * 
 * ПОШАГОВОЕ ОБЪЯСНЕНИЕ РЕШЕНИЯ:
 * 1. Вариант 1 (Целочисленное деление):
 *    int resInt = a / b;
 *    Так как оба операнда 'a' и 'b' имеют целочисленный тип (int), компилятор
 *    выполняет целочисленное деление (integer division).
 *    Дробная часть безвозвратно отбрасывается: 7 / 3 = 2 (остаток 1 отбрасывается).
 * 
 * 2. Вариант 2 (Деление с явным приведением к double):
 *    double resDouble = (double)a / b; 
 *    (или в стиле C++: static_cast<double>(a) / b)
 *    Здесь мы явно приводим операнд 'a' к типу 'double'.
 *    По правилам арифметических преобразований C++ (Usual Arithmetic Conversions):
 *    если один операнд double, второй операнд 'b' неявно преобразуется к double,
 *    и деление выполняется как вещественное: 7.0 / 3.0 ≈ 2.3333333333333335.
 * 
 * 3. Форматирование и округление до 3 знаков после запятой:
 *    - Для вывода используются манипуляторы:
 *      std::fixed — фиксированный формат отображения вещественных чисел;
 *      std::setprecision(3) — ровно 3 знака после десятичной точки.
 *    - Для математического округления значения можно применить round(x * 1000.0) / 1000.0.
 */

#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ЗАДАЧА 4: Преобразование типов (явное и неявное) ===" << std::endl;

    // Исходные данные
    int a = 7;
    int b = 3;

    std::cout << "[Исходные данные]: int a = " << a << "; int b = " << b << ";" << std::endl;

    // ВАРИАНТ 1: Целочисленное деление без приведения типов
    int intDivResult = a / b;
    std::cout << "\n1. Целочисленное деление (a / b):" << std::endl;
    std::cout << "   Результат: " << intDivResult 
              << " (тип int, дробная часть 0.333... отброшена)" << std::endl;

    // ВАРИАНТ 2: Явное приведение типов к double
    // Способ A: C-style cast -> (double)a
    double resCStyle = (double)a / b;

    // Способ B: Рекомендуемый C++ static_cast -> static_cast<double>(a)
    double resCppStyle = static_cast<double>(a) / b;

    std::cout << "\n2. Деление с явным приведением типа double:" << std::endl;
    std::cout << "   (double)a / b:              " << resCStyle << std::endl;
    std::cout << "   static_cast<double>(a) / b: " << resCppStyle << std::endl;

    // Округление и вывод до 3 знаков после запятой
    std::cout << "\n3. Округление и вывод ровно до 3 знаков после запятой:" << std::endl;
    std::cout << "   Через std::fixed и std::setprecision(3): ";
    std::cout << std::fixed << std::setprecision(3) << resCppStyle << std::endl;

    // Математическое округление
    double roundedMath = std::round(resCppStyle * 1000.0) / 1000.0;
    std::cout << "   Математически округленное значение:       " << roundedMath << std::endl;

    return 0;
}

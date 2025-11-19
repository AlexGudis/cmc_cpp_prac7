#include "func.h"
#include <iostream>

/**
 * Пример использования библиотеки (демонстрация работы)
 * Соответствует примеру из задания
 */
int main() {
    try {
        // Пример из задания
        std::vector<TFunctionPtr> cont;
        
        // Создаем степенную функцию x^2
        auto f = FunctionFactory::Create("power", 2);
        cont.push_back(f);
        
        // Создаем полиномиальную функцию: 7 + 3*x^2 + 15*x^3
        auto g = FunctionFactory::Create("polynomial", std::vector<double>{7, 0, 3, 15});
        cont.push_back(g);
        
        // Выводим значения функций в точке x=10
        for (const auto& ptr : cont) {
            std::cout << ptr->ToString() << " for x = 10 is " << (*ptr)(10) << std::endl;
        }
        
        // Создаем сумму функций и вычисляем производную в точке x=1
        auto p = *f + *g;
        std::cout << "Derivative of (" << f->ToString() << " + " << g->ToString() 
                  << ") at x=1 is " << p->GetDeriv(1) << std::endl; // Должно быть 53
        
        // Тестирование градиентного спуска для нахождения корня x^2 - 4 = 0
        auto quadratic = FunctionFactory::Create("polynomial", std::vector<double>{-4, 0, 1});
        double root = FindRootGradientDescent(*quadratic, 1.0, 100);
        std::cout << "Root of x^2 - 4 found at: " << root << std::endl;
        
    } catch (const std::exception& e) {
        // Обработка исключений (например, деление на ноль или неизвестный тип функции)
        std::cerr << "Error: " << e.what() << std::endl;
    }


    try {
        // Создаем базовые функции
        auto x = FunctionFactory::Create("ident");           // f(x) = x
        auto x2 = FunctionFactory::Create("power", 2);         // f(x) = x^2
        auto c5 = FunctionFactory::Create("const", 5);      // f(x) = 5
        auto poly = FunctionFactory::Create("polynomial", {1, 2, 3}); // f(x) = 1 + 2x + 3x^2
        
        // Демонстрация цепочек операций (как требует задание)
        std::cout << "=== Цепочки операций ===" << std::endl;
        
        // Простая операция: (x^2 + 5)
        auto expr1 = *x2 + *c5;
        std::cout << "expr1 = " << expr1->ToString() << std::endl;
        std::cout << "expr1(2) = " << (*expr1)(2) << " (ожидается: 4 + 5 = 9)" << std::endl;
        std::cout << "expr1'(2) = " << expr1->GetDeriv(2) << " (ожидается: 4)" << std::endl;
        
        // Более сложная цепочка: ((x^2 + 5) * (1 + 2x + 3x^2))
        auto expr2 = *expr1 * *poly;
        std::cout << "\nexpr2 = " << expr2->ToString() << std::endl;
        std::cout << "expr2(2) = " << (*expr2)(2) << " (ожидается: 9 * 17 = 153)" << std::endl;
        
        // Еще более сложная цепочка: (((x^2 + 5) * (1 + 2x + 3x^2)) - x)
        auto expr3 = *expr2 - *x;
        std::cout << "\nexpr3 = " << expr3->ToString() << std::endl;
        std::cout << "expr3(2) = " << (*expr3)(2) << " (ожидается: 153 - 2 = 151)" << std::endl;
        
        // Деление: ((x^2 + 5) / x)
        auto expr4 = *x2 / *x;
        std::cout << "\nexpr4 = " << expr4->ToString() << std::endl;
        std::cout << "expr4(2) = " << (*expr4)(2) << " (ожидается: 4 / 2 = 2)" << std::endl;
        
        // Очень сложное выражение: (((x^2 + 5) * (1 + 2x + 3x^2)) - x) / x^2
        auto expr5 = *expr3 / *x2;
        std::cout << "\nexpr5 = " << expr5->ToString() << std::endl;
        std::cout << "expr5(2) = " << (*expr5)(2) << " (ожидается: 151 / 4 = 37.75)" << std::endl;
        
        // Проверка производной сложного выражения
        std::cout << "\n=== Проверка производных ===" << std::endl;
        std::cout << "Производная expr1 в точке 2: " << expr1->GetDeriv(2) << std::endl;
        std::cout << "Производная expr2 в точке 2: " << expr2->GetDeriv(2) << std::endl;
        std::cout << "Производная expr3 в точке 2: " << expr3->GetDeriv(2) << std::endl;
        
        // Градиентный спуск с сложной функцией
        std::cout << "\n=== Градиентный спуск ===" << std::endl;
        // Ищем корень уравнения: (x^2 - 4) = 0 (корни: x=2, x=-2)
        auto quadratic = *x2 - *FunctionFactory::Create("constant", 4);
        double root = FindRootGradientDescent(*quadratic, 1.0, 100);
        std::cout << "Корень уравнения x^2 - 4 = 0: " << root << std::endl;
        
        // Более сложное уравнение: (x^2 + 2x - 8) = 0 (корни: x=2, x=-4)
        //auto complex_eq = *x2 + *(*FunctionFactory::Create("const", 2) * *x) - *(FunctionFactory::Create("const", 8));
        //root = FindRootGradientDescent(*complex_eq, 1.0, 100);
        //std::cout << "Корень уравнения x^2 + 2x - 8 = 0: " << root << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    
    return 0;
}
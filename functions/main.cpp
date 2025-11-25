#include "func.h"
#include <iostream>

int main() {
    try {
        // std::vector<TFunctionPtr> cont;
        
        // // f(x) = x^2
        // auto f = FunctionFactory::Create("power", 2);
        // cont.push_back(f);
        
        // // g(x) =  7 + 3*x^2 + 15*x^3
        // auto g = FunctionFactory::Create("polynomial", std::vector<double>{7, 0, 3, 15});
        // cont.push_back(g);
        
        // for (const auto& ptr : cont) {
        //     std::cout << ptr->ToString() << " for x = 10 is " << (*ptr)(10) << std::endl;
        // }
        
        // auto p = *f + *g;
        // std::cout << "Derivative of (" << f->ToString() << " + " << g->ToString() 
        //           << ") at x=1 is " << p->GetDeriv(1) << std::endl; // 53
        
        // // x^2 - 4 = 0
        // auto quadratic = FunctionFactory::Create("polynomial", std::vector<double>{-4, 0, 1});
        // double root = FindRootGradientDescent(*quadratic, 1.0, 100);
        // std::cout << "Root of x^2 - 4 found at: " << root << std::endl;
        


        // auto x = FunctionFactory::Create("ident");             // f(x) = x
        // auto x2 = FunctionFactory::Create("power", 2);         // f(x) = x^2
        // auto c5 = FunctionFactory::Create("const", 5);         // f(x) = 5
        // auto poly = FunctionFactory::Create("polynomial", {1, 2, 3}); // f(x) = 1 + 2x + 3x^2
        
        // std::cout << "=== Цепочки операций ===" << std::endl;
        
        // // (x^2 + 5)
        // auto expr1 = *x2 + *c5;
        // std::cout << "expr1 = " << expr1->ToString() << std::endl;
        // std::cout << "expr1(2) = " << (*expr1)(2) << " (ожидается: 4 + 5 = 9)" << std::endl;
        // std::cout << "expr1'(2) = " << expr1->GetDeriv(2) << " (ожидается: 4)" << std::endl;
        
        // // ((x^2 + 5) * (1 + 2x + 3x^2))
        // auto expr2 = *expr1 * *poly;
        // std::cout << "\nexpr2 = " << expr2->ToString() << std::endl;
        // std::cout << "expr2(2) = " << (*expr2)(2) << " (ожидается: 9 * 17 = 153)" << std::endl;
        
        // // ((x^2 + 5) / x)
        // auto expr4 = *x2 / *x;
        // std::cout << "\nexpr4 = " << expr4->ToString() << std::endl;
        // std::cout << "expr4(2) = " << (*expr4)(2) << " (ожидается: 4 / 2 = 2)" << std::endl;
        
        // std::cout << "\n=== Проверка производных ===" << std::endl;
        // std::cout << "Производная expr1 в точке 2: " << expr1->GetDeriv(2) << std::endl;
        // std::cout << "Производная expr2 в точке 2: " << expr2->GetDeriv(2) << std::endl;
        

        // (x + 2) * (x - 3)
        auto g1 = FunctionFactory::Create("polynomial", std::vector<double>{2, 1});
        auto g2 = FunctionFactory::Create("polynomial", std::vector<double>{-3, 1});

        auto f = (*g1) * (*g2);
        std::cout << f->ToString() << std::endl;
        std::cout << (*f)(5) << std::endl;
        std::cout << f->GetDeriv(10) << std::endl;
        std::cout << FindRootGradientDescent(*f, 17, 10000) << std::endl;


    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    
    return 0;
}
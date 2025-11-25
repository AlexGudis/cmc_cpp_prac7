#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "func.h"

using ::testing::DoubleNear;
using ::testing::ThrowsMessage;

class FunctionLibraryTest : public ::testing::Test {
protected:
    void SetUp() override {
        ident = FunctionFactory::Create("ident");           // f(x) = x
        constant = FunctionFactory::Create("const", 5.0);   // f(x) = 5
        power = FunctionFactory::Create("power", 3);        // f(x) = x³
        exp_func = FunctionFactory::Create("exp");          // f(x) = exp(x)
        poly = FunctionFactory::Create("polynomial", std::vector<double>{1, 2, 3}); // 1 + 2x + 3x^2
    }

    TFunctionPtr ident;
    TFunctionPtr constant;
    TFunctionPtr power;
    TFunctionPtr exp_func;
    TFunctionPtr poly;
};

// Проверка тождественной функции
TEST_F(FunctionLibraryTest, CreateIdentFunction) {
    EXPECT_EQ(ident->ToString(), "x");        
    EXPECT_EQ((*ident)(2.0), 2.0);            
    EXPECT_EQ(ident->GetDeriv(2.0), 1.0);     
}

// Проверка константной функции  
TEST_F(FunctionLibraryTest, CreateConstFunction) {
    EXPECT_EQ(constant->ToString(), "5");    
    EXPECT_EQ((*constant)(2.0), 5.0);         
    EXPECT_EQ(constant->GetDeriv(2.0), 0.0);  
}

// Проверка степенной функции
TEST_F(FunctionLibraryTest, CreatePowerFunction) {
    EXPECT_EQ(power->ToString(), "x^3");      // "x^3"
    EXPECT_EQ((*power)(2.0), 8.0);
    EXPECT_EQ(power->GetDeriv(2.0), 12.0);
}

// Проверка экспоненты
TEST_F(FunctionLibraryTest, CreateExpFunction) {
    EXPECT_EQ(exp_func->ToString(), "exp(x)");
    EXPECT_DOUBLE_EQ((*exp_func)(0.0), 1.0);
    EXPECT_DOUBLE_EQ(exp_func->GetDeriv(0.0), 1.0);
}

// Проверка полинома
TEST_F(FunctionLibraryTest, CreatePolynomialFunction) {
    EXPECT_EQ(poly->ToString(), "1 + 2*x + 3*x^2");
    EXPECT_EQ((*poly)(1.0), 6.0);             // 1 + 2*1 + 3*1 = 6
    EXPECT_EQ(poly->GetDeriv(1.0), 8.0);      // 2 + 6x, при x=1: 2+6=8
}

// Проверка обработки ошибок
TEST_F(FunctionLibraryTest, InvalidFunctionTypeThrows) {
    EXPECT_THROW(FunctionFactory::Create("unknown"), std::logic_error);
}

// Проверка оператора сложения
TEST_F(FunctionLibraryTest, AddFunctions) {
    auto sum = *ident + *constant;            // x + 5
    EXPECT_EQ((*sum)(2.0), 7.0);              // 2 + 5 = 7
    EXPECT_EQ(sum->GetDeriv(2.0), 1.0);       // Производная: 1 + 0 = 1
}

// Проверка оператора вычитания
TEST_F(FunctionLibraryTest, SubtractFunctions) {
    auto diff = *power - *constant;           // x^3 - 5
    EXPECT_EQ((*diff)(2.0), 3.0);             // 8 - 5 = 3
    EXPECT_EQ(diff->GetDeriv(2.0), 12.0);     // 3x^2 - 0 = 12
}

// Проверка оператора умножения
TEST_F(FunctionLibraryTest, MultiplyFunctions) {
    auto product = *ident * *constant;        // x * 5
    EXPECT_EQ((*product)(2.0), 10.0);         // 2 * 5 = 10
    EXPECT_EQ(product->GetDeriv(2.0), 5.0);   // Производная: 1*5 + x*0 = 5
}

// Проверка оператора деления
TEST_F(FunctionLibraryTest, DivideFunctions) {
    auto quotient = *power / *constant;       // x^3 / 5
    EXPECT_EQ((*quotient)(2.0), 1.6);         // 8 / 5 = 1.6
    EXPECT_DOUBLE_EQ(quotient->GetDeriv(2.0), 12.0 / 5.0); // Производная: (3x^2)/5 = 12/5
}

// Проверка сложных выражений
TEST_F(FunctionLibraryTest, ComplexExpression) {
    // f(x) = (x² + 2x - 1) * exp(x)
    auto poly2 = FunctionFactory::Create("polynomial", std::vector<double>{-1, 2, 1});
    auto complex = *poly2 * *exp_func;
    
    EXPECT_DOUBLE_EQ((*complex)(0.0), -1.0);  // (-1 + 0 + 0) * 1 = -1
}

// Проверка производных
TEST_F(FunctionLibraryTest, DerivativeOfPower) {
    auto power2 = FunctionFactory::Create("power", 2); // x^2
    EXPECT_EQ(power2->GetDeriv(3.0), 6.0);    // 2x, при x=3: 6
}

// Проверка градиентного спуска для линейной функции
TEST_F(FunctionLibraryTest, GradientDescentLinear) {
    auto linear = FunctionFactory::Create("polynomial", std::vector<double>{-2, 1}); // f(x) = - 2 + x
    double root = FindRootGradientDescent(*linear, 0.0, 100);
    EXPECT_NEAR(root, 2.0, 0.1);           
}

// Проверка обработки исключений
TEST_F(FunctionLibraryTest, DivisionByZeroThrows) {
    auto zero = FunctionFactory::Create("const", 0.0);
    auto div = *constant / *zero;             // 5 / 0
    
    EXPECT_THROW((*div)(1.0), std::logic_error);
    EXPECT_THROW(div->GetDeriv(1.0), std::logic_error); 
}

TEST_F(FunctionLibraryTest, AssignmentExample) {
    // f = x^2, g = 7 + 3 * x^2 + 15 * x^3
    auto f = FunctionFactory::Create("power", 2);
    auto g = FunctionFactory::Create("polynomial", std::vector<double>{7, 0, 3, 15});
    
    auto p = *f + *g; // x² + (7 + 3x² + 15x³)
    
    // Проверяем производную в точке x=1
    // f'(x) = 2x
    // g'(x) = 6x + 45 * x^2, g'(1) = 51  
    // p'(1) = 2 + 51 = 53
    EXPECT_EQ(p->GetDeriv(1), 53);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();                
}
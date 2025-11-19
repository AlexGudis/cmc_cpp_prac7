#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "func.h"

using ::testing::DoubleNear;
using ::testing::ThrowsMessage;

class FunctionLibraryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Используем статический метод Create через ::
        ident = FunctionFactory::Create("ident");
        constant = FunctionFactory::Create("const", 5.0);
        power = FunctionFactory::Create("power", 3);
        exp_func = FunctionFactory::Create("exp");
        poly = FunctionFactory::Create("polynomial", std::vector<double>{1, 2, 3}); // 1 + 2x + 3x^2
    }

    TFunctionPtr ident;
    TFunctionPtr constant;
    TFunctionPtr power;
    TFunctionPtr exp_func;
    TFunctionPtr poly;
};

// Тесты создания функций
TEST_F(FunctionLibraryTest, CreateIdentFunction) {
    EXPECT_EQ(ident->ToString(), "x");
    EXPECT_EQ((*ident)(2.0), 2.0);
    EXPECT_EQ(ident->GetDeriv(2.0), 1.0);
}

TEST_F(FunctionLibraryTest, CreateConstFunction) {
    EXPECT_EQ(constant->ToString(), "5");
    EXPECT_EQ((*constant)(2.0), 5.0);
    EXPECT_EQ(constant->GetDeriv(2.0), 0.0);
}

TEST_F(FunctionLibraryTest, CreatePowerFunction) {
    EXPECT_EQ(power->ToString(), "x^3");
    EXPECT_EQ((*power)(2.0), 8.0);
    EXPECT_EQ(power->GetDeriv(2.0), 12.0); // 3*2^2 = 12
}

TEST_F(FunctionLibraryTest, CreateExpFunction) {
    EXPECT_EQ(exp_func->ToString(), "exp(x)");
    EXPECT_DOUBLE_EQ((*exp_func)(0.0), 1.0);
    EXPECT_DOUBLE_EQ(exp_func->GetDeriv(0.0), 1.0);
}

TEST_F(FunctionLibraryTest, CreatePolynomialFunction) {
    EXPECT_EQ(poly->ToString(), "1 + 2*x + 3*x^2");
    EXPECT_EQ((*poly)(1.0), 6.0); // 1 + 2*1 + 3*1 = 6
    EXPECT_EQ(poly->GetDeriv(1.0), 8.0); // 2 + 6*1 = 8
}

TEST_F(FunctionLibraryTest, InvalidFunctionTypeThrows) {
    EXPECT_THROW(FunctionFactory::Create("unknown"), std::logic_error);
}

// Тесты арифметических операций
TEST_F(FunctionLibraryTest, AddFunctions) {
    auto sum = *ident + *constant;
    EXPECT_EQ((*sum)(2.0), 7.0); // 2 + 5 = 7
    EXPECT_EQ(sum->GetDeriv(2.0), 1.0); // производная x + 5 = 1
}

TEST_F(FunctionLibraryTest, SubtractFunctions) {
    auto diff = *power - *constant;
    EXPECT_EQ((*diff)(2.0), 3.0); // 8 - 5 = 3
    EXPECT_EQ(diff->GetDeriv(2.0), 12.0); // производная x^3 - 5 = 3x^2
}

TEST_F(FunctionLibraryTest, MultiplyFunctions) {
    auto product = *ident * *constant;
    EXPECT_EQ((*product)(2.0), 10.0); // 2 * 5 = 10
    EXPECT_EQ(product->GetDeriv(2.0), 5.0); // производная 5x = 5
}

TEST_F(FunctionLibraryTest, DivideFunctions) {
    auto quotient = *power / *constant;
    EXPECT_EQ((*quotient)(2.0), 1.6); // 8 / 5 = 1.6
    EXPECT_DOUBLE_EQ(quotient->GetDeriv(2.0), 12.0 / 5.0); // производная (x^3)/5 = (3x^2)/5
}

TEST_F(FunctionLibraryTest, ComplexExpression) {
    // f(x) = (x^2 + 2x - 1) * exp(x)
    auto poly2 = FunctionFactory::Create("polynomial", std::vector<double>{-1, 2, 1}); // -1 + 2x + x^2
    auto complex = *poly2 * *exp_func;
    
    EXPECT_DOUBLE_EQ((*complex)(0.0), -1.0); // (-1)*1 = -1
}

// Тесты производных
TEST_F(FunctionLibraryTest, DerivativeOfPower) {
    auto power2 = FunctionFactory::Create("power", 2);
    EXPECT_EQ(power2->GetDeriv(3.0), 6.0); // (x^2)' = 2x, 2*3=6
}

TEST_F(FunctionLibraryTest, DerivativeOfPolynomial) {
    // 1 + 2x + 3x^2, производная = 2 + 6x
    EXPECT_EQ(poly->GetDeriv(2.0), 14.0); // 2 + 6*2 = 14
}

TEST_F(FunctionLibraryTest, DerivativeOfComplexFunction) {
    // f(x) = x^3 + 2x, производная = 3x^2 + 2
    auto linear = FunctionFactory::Create("polynomial", std::vector<double>{0, 2}); // 2x
    auto func = *power + *linear;
    
    EXPECT_EQ(func->GetDeriv(1.0), 5.0); // 3*1 + 2 = 5
    EXPECT_EQ(func->GetDeriv(2.0), 14.0); // 3*4 + 2 = 14
}

// Тесты градиентного спуска
TEST_F(FunctionLibraryTest, GradientDescentLinear) {
    // f(x) = x - 2, корень x = 2
    auto linear = FunctionFactory::Create("polynomial", std::vector<double>{-2, 1}); // x - 2
    double root = FindRootGradientDescent(*linear, 0.0, 100);
    EXPECT_NEAR(root, 2.0, 0.1);
}

TEST_F(FunctionLibraryTest, GradientDescentQuadratic) {
    // f(x) = x^2 - 4, корни x = ±2
    auto quadratic = FunctionFactory::Create("polynomial", std::vector<double>{-4, 0, 1}); // x^2 - 4
    double root = FindRootGradientDescent(*quadratic, 1.0, 100);
    EXPECT_NEAR(root, 2.0, 0.1);
}

// Тесты исключений
TEST_F(FunctionLibraryTest, DivisionByZeroThrows) {
    auto zero = FunctionFactory::Create("const", 0.0);
    auto div = *constant / *zero;
    
    EXPECT_THROW((*div)(1.0), std::logic_error);
    EXPECT_THROW(div->GetDeriv(1.0), std::logic_error);
}

TEST_F(FunctionLibraryTest, InvalidParametersThrow) {
    EXPECT_THROW(FunctionFactory::Create("const"), std::logic_error);
    EXPECT_THROW(FunctionFactory::Create("power"), std::logic_error);
}

// Тесты строкового представления
TEST_F(FunctionLibraryTest, StringRepresentation) {
    EXPECT_EQ(ident->ToString(), "x");
    EXPECT_EQ(constant->ToString(), "5");
    EXPECT_EQ(power->ToString(), "x^3");
    EXPECT_EQ(exp_func->ToString(), "exp(x)");
    
    auto poly_simple = FunctionFactory::Create("polynomial", std::vector<double>{1});
    EXPECT_EQ(poly_simple->ToString(), "1");
    
    auto poly_complex = FunctionFactory::Create("polynomial", std::vector<double>{0, -1, 0, 2});
    EXPECT_EQ(poly_complex->ToString(), "-x + 2*x^3");
}

// Тест из задания - проверяем конкретный пример
TEST_F(FunctionLibraryTest, AssignmentExample) {
    // Пример из задания: f = x^2, g = 7 + 3*x^2 + 15*x^3
    auto f = FunctionFactory::Create("power", 2);
    auto g = FunctionFactory::Create("polynomial", std::vector<double>{7, 0, 3, 15});
    
    auto p = *f + *g;
    
    // Проверяем производную в точке x=1
    // f(x) = x^2, f'(x) = 2x, f'(1) = 2
    // g(x) = 7 + 3x^2 + 15x^3, g'(x) = 6x + 45x^2, g'(1) = 6 + 45 = 51
    // p'(1) = f'(1) + g'(1) = 2 + 51 = 53
    EXPECT_EQ(p->GetDeriv(1), 53);
}

// Дополнительные тесты для полного покрытия
TEST_F(FunctionLibraryTest, PowerFunctionEdgeCases) {
    auto power0 = FunctionFactory::Create("power", 0);
    EXPECT_EQ(power0->ToString(), "1");
    EXPECT_EQ((*power0)(5.0), 1.0);
    EXPECT_EQ(power0->GetDeriv(5.0), 0.0);
    
    auto power1 = FunctionFactory::Create("power", 1);
    EXPECT_EQ(power1->ToString(), "x");
    EXPECT_EQ((*power1)(5.0), 5.0);
    EXPECT_EQ(power1->GetDeriv(5.0), 1.0);
}

TEST_F(FunctionLibraryTest, EmptyPolynomial) {
    auto empty_poly = FunctionFactory::Create("polynomial", std::vector<double>{});
    EXPECT_EQ(empty_poly->ToString(), "0");
    EXPECT_EQ((*empty_poly)(5.0), 0.0);
    EXPECT_EQ(empty_poly->GetDeriv(5.0), 0.0);
}

// Главная функция для тестов
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
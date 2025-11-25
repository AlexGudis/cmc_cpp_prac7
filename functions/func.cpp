#include "func.h"
#include <cmath>
#include <sstream>
#include <iostream>


// --------------------------------------------------- Реализация ---------------------------------------------------
// IdentFunction implementation
// Вычисляет значение тождественной функции f(x) = x
double IdentFunction::operator()(double x) const {
    return x;
}

// Вычисляет производную тождественной функции
double IdentFunction::GetDeriv(double x) const {
    return 1.0;
}

std::string IdentFunction::ToString() const {
    return "x";
}

std::shared_ptr<TFunction> IdentFunction::Clone() const {
    return std::make_shared<IdentFunction>();
}


// ConstFunction implementation
// value - значение константы
ConstFunction::ConstFunction(double value) : value_(value) {}

// Вычисляет значение константной функции f(x) = c
double ConstFunction::operator()(double x) const {
    return value_;
}

double ConstFunction::GetDeriv(double x) const {
    return 0.0;  // Производная константы
}

std::string ConstFunction::ToString() const {
    std::ostringstream oss;
    oss << value_;
    return oss.str();
}

std::shared_ptr<TFunction> ConstFunction::Clone() const {
    return std::make_shared<ConstFunction>(value_);
}


// PowerFunction implementation
/**
 * Конструктор степенной функции: x**power
 * power - показатель степени
 */
PowerFunction::PowerFunction(int power) : power_(power) {}

// Вычисляет значение степенной функции f(x) = x^n
double PowerFunction::operator()(double x) const {
    return std::pow(x, power_);
}

// Вычисляет производную степенной функции
double PowerFunction::GetDeriv(double x) const {
    if (power_ == 0) return 0.0;  // Производная константы
    return power_ * std::pow(x, power_ - 1);
}

std::string PowerFunction::ToString() const {
    if (power_ == 0) return "1";      // x^0 = 1
    if (power_ == 1) return "x";      // x^1 = x
    return "x^" + std::to_string(power_);  // x^n для n>1
}

std::shared_ptr<TFunction> PowerFunction::Clone() const {
    return std::make_shared<PowerFunction>(power_);
}


// ExpFunction implementation
// Вычисляет значение экспоненциальной функции f(x) = exp(x)
double ExpFunction::operator()(double x) const {
    return std::exp(x);
}

double ExpFunction::GetDeriv(double x) const {
    return std::exp(x);  // Производная экспоненты равна самой экспоненте
}

std::string ExpFunction::ToString() const {
    return "exp(x)";
}

std::shared_ptr<TFunction> ExpFunction::Clone() const {
    return std::make_shared<ExpFunction>();
}


// PolynomialFunction implementation
// Конструктор полиномиальной функции
PolynomialFunction::PolynomialFunction(const std::vector<double>& coefficients) 
    : coefficients_(coefficients) {}

// Вычисляет значение полинома в точке x
double PolynomialFunction::operator()(double x) const {
    double result = 0.0;
    double x_power = 1.0;
    
    for (double coef : coefficients_) {
        result += coef * x_power;
        x_power *= x;
    }
    
    return result;
}

// Вычисляет производную полинома в точке x: a_1 + 2 * a_2 * x + 3 * a_3 * x^2 + ... + n * a_n * x^(n-1)
double PolynomialFunction::GetDeriv(double x) const {
    if (coefficients_.size() <= 1) return 0.0;
    
    double result = 0.0;
    double x_power = 1.0;
    for (size_t i = 1; i < coefficients_.size(); ++i) {
        result += coefficients_[i] * i * x_power;
        x_power *= x;
    }
    
    return result;
}

// Форматирует полином в читаемом виде: "1 + 2*x - 3*x^2"
std::string PolynomialFunction::ToString() const {
    if (coefficients_.empty()) return "0";
    
    std::ostringstream oss;
    bool first_term = true;
    
    for (size_t i = 0; i < coefficients_.size(); ++i) {
        if (coefficients_[i] == 0) continue; 
        
        // Обрабатываем знак
        if (!first_term) {
            oss << (coefficients_[i] > 0 ? " + " : " - ");
        } else if (coefficients_[i] < 0) {
            oss << "-";
        }
        
        double abs_coef = std::abs(coefficients_[i]);
        
        // Форматируем член в зависимости от степени
        if (i == 0) {
            oss << abs_coef;
        } else {
            if (abs_coef != 1.0) {
                oss << abs_coef << "*";
            }
            oss << "x";
            if (i > 1) {
                oss << "^" << i;
            }
        }
        
        first_term = false;
    }
    
    return oss.str();
}

std::shared_ptr<TFunction> PolynomialFunction::Clone() const {
    return std::make_shared<PolynomialFunction>(coefficients_);
}


// AddFunction implementation
AddFunction::AddFunction(TFunctionPtr left, TFunctionPtr right) 
    : left_(left), right_(right) {}

// Вычисляет сумму двух функций: f(x) = g(x) + h(x)
double AddFunction::operator()(double x) const {
    return (*left_)(x) + (*right_)(x);
}

// Производная суммы: (g+h)' = g' + h'
double AddFunction::GetDeriv(double x) const {
    return left_->GetDeriv(x) + right_->GetDeriv(x);
}

std::string AddFunction::ToString() const {
    return "(" + left_->ToString() + " + " + right_->ToString() + ")";
}

std::shared_ptr<TFunction> AddFunction::Clone() const {
    return std::make_shared<AddFunction>(left_->Clone(), right_->Clone());
}


// SubtractFunction implementation
SubtractFunction::SubtractFunction(TFunctionPtr left, TFunctionPtr right) 
    : left_(left), right_(right) {}

// Вычисляет разность двух функций: f(x) = g(x) - h(x)
double SubtractFunction::operator()(double x) const {
    return (*left_)(x) - (*right_)(x);
}

// Производная разности: (g-h)' = g' - h'
double SubtractFunction::GetDeriv(double x) const {
    return left_->GetDeriv(x) - right_->GetDeriv(x);
}

std::string SubtractFunction::ToString() const {
    return "(" + left_->ToString() + " - " + right_->ToString() + ")";
}

std::shared_ptr<TFunction> SubtractFunction::Clone() const {
    return std::make_shared<SubtractFunction>(left_->Clone(), right_->Clone());
}


// MultiplyFunction implementation
MultiplyFunction::MultiplyFunction(TFunctionPtr left, TFunctionPtr right) 
    : left_(left), right_(right) {}

// Вычисляет произведение двух функций: f(x) = g(x) * h(x)
double MultiplyFunction::operator()(double x) const {
    return (*left_)(x) * (*right_)(x);
}

double MultiplyFunction::GetDeriv(double x) const {
    // (f*g)' = f'*g + f*g'
    return left_->GetDeriv(x) * (*right_)(x) + (*left_)(x) * right_->GetDeriv(x);
}

std::string MultiplyFunction::ToString() const {
    return "(" + left_->ToString() + " * " + right_->ToString() + ")";
}

std::shared_ptr<TFunction> MultiplyFunction::Clone() const {
    return std::make_shared<MultiplyFunction>(left_->Clone(), right_->Clone());
}


// DivideFunction implementation
DivideFunction::DivideFunction(TFunctionPtr left, TFunctionPtr right) 
    : left_(left), right_(right) {}

// Вычисляет частное двух функций: f(x) = g(x) / h(x)
double DivideFunction::operator()(double x) const {
    double denominator = (*right_)(x);
    if (denominator == 0) {
        throw std::logic_error("Division by zero");
    }
    return (*left_)(x) / denominator;
}

double DivideFunction::GetDeriv(double x) const {
    // (f/g)' = (f'*g - f*g') / g^2
    double f = (*left_)(x); 
    double g = (*right_)(x);    
    double f_prime = left_->GetDeriv(x);  
    double g_prime = right_->GetDeriv(x); 
    
    if (g == 0) {
        throw std::logic_error("Division by zero in derivative");  
    }
    
    return (f_prime * g - f * g_prime) / (g * g);  
}

std::string DivideFunction::ToString() const {
    return "(" + left_->ToString() + " / " + right_->ToString() + ")";
}

std::shared_ptr<TFunction> DivideFunction::Clone() const {
    return std::make_shared<DivideFunction>(left_->Clone(), right_->Clone());
}


// FunctionFactory implementation
TFunctionPtr FunctionFactory::Create(const std::string& type, const std::vector<double>& params) {
    if (type == "ident") {
        return std::make_shared<IdentFunction>();
    } else if (type == "const") {
        if (params.size() != 1) {
            throw std::logic_error("Const function requires exactly one parameter");
        }
        return std::make_shared<ConstFunction>(params[0]);
    } else if (type == "power") {
        if (params.size() != 1) {
            throw std::logic_error("Power function requires exactly one parameter");
        }
        return std::make_shared<PowerFunction>(static_cast<int>(params[0]));
    } else if (type == "exp") {
        return std::make_shared<ExpFunction>();
    } else if (type == "polynomial") {
        return std::make_shared<PolynomialFunction>(params);
    } else {
        throw std::logic_error("Unknown function type: " + type);
    }
}

// Перегруженная версия для создания функций с одним double параметром
TFunctionPtr FunctionFactory::Create(const std::string& type, double param) {
    return Create(type, std::vector<double>{param});
}

TFunctionPtr FunctionFactory::Create(const std::string& type, int param) {
    return Create(type, std::vector<double>{static_cast<double>(param)});  // Преобразуем int в double
}


// Operator overloads
TFunctionPtr operator+(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<AddFunction>(lhs.Clone(), rhs.Clone());
}

TFunctionPtr operator-(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<SubtractFunction>(lhs.Clone(), rhs.Clone());
}

TFunctionPtr operator*(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<MultiplyFunction>(lhs.Clone(), rhs.Clone());
}

TFunctionPtr operator/(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<DivideFunction>(lhs.Clone(), rhs.Clone());
}


// Gradient descent implementation
// x_{n+1} = x_n - learning_rate * f(x_n) / f'(x_n)

double FindRootGradientDescent(const TFunction& func, double initial_guess, int iterations) {
    double x = initial_guess;
    double learning_rate = 0.1;
    
    for (int i = 0; i < iterations; ++i) {
        double gradient = func.GetDeriv(x); 
        double value = func(x);          
        
        // Добавляем 1e-10 для избежания деления на ноль
        x = x - learning_rate * value / (std::abs(gradient) + 1e-10);
    }
    
    return x;
}


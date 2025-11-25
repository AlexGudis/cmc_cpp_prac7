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
/**
 * Конструктор функции-суммы
 * @param left - левая функция (первое слагаемое)
 * @param right - правая функция (второе слагаемое)
 */
AddFunction::AddFunction(TFunctionPtr left, TFunctionPtr right) 
    : left_(left), right_(right) {}

/**
 * Вычисляет сумму двух функций: f(x) = g(x) + h(x)
 */
double AddFunction::operator()(double x) const {
    return (*left_)(x) + (*right_)(x);  // Сумма значений
}

/**
 * Вычисляет производную суммы
 * Производная суммы: (g+h)' = g' + h'
 */
double AddFunction::GetDeriv(double x) const {
    return left_->GetDeriv(x) + right_->GetDeriv(x);  // Сумма производных
}

/**
 * Строковое представление суммы
 * Формат: "(left + right)"
 */
std::string AddFunction::ToString() const {
    return "(" + left_->ToString() + " + " + right_->ToString() + ")";
}

/**
 * Создает копию функции-суммы
 */
std::shared_ptr<TFunction> AddFunction::Clone() const {
    return std::make_shared<AddFunction>(left_->Clone(), right_->Clone());
}


// SubtractFunction implementation
/**
 * Конструктор функции-разности
 * @param left - левая функция (уменьшаемое)
 * @param right - правая функция (вычитаемое)
 */
SubtractFunction::SubtractFunction(TFunctionPtr left, TFunctionPtr right) 
    : left_(left), right_(right) {}

/**
 * Вычисляет разность двух функций: f(x) = g(x) - h(x)
 */
double SubtractFunction::operator()(double x) const {
    return (*left_)(x) - (*right_)(x);  // Разность значений
}

/**
 * Вычисляет производную разности
 * Производная разности: (g-h)' = g' - h'
 */
double SubtractFunction::GetDeriv(double x) const {
    return left_->GetDeriv(x) - right_->GetDeriv(x);  // Разность производных
}

/**
 * Строковое представление разности
 * Формат: "(left - right)"
 */
std::string SubtractFunction::ToString() const {
    return "(" + left_->ToString() + " - " + right_->ToString() + ")";
}

/**
 * Создает копию функции-разности
 */
std::shared_ptr<TFunction> SubtractFunction::Clone() const {
    return std::make_shared<SubtractFunction>(left_->Clone(), right_->Clone());
}


// MultiplyFunction implementation
/**
 * Конструктор функции-произведения
 * @param left - левая функция (первый множитель)
 * @param right - правая функция (второй множитель)
 */
MultiplyFunction::MultiplyFunction(TFunctionPtr left, TFunctionPtr right) 
    : left_(left), right_(right) {}

/**
 * Вычисляет произведение двух функций: f(x) = g(x) * h(x)
 */
double MultiplyFunction::operator()(double x) const {
    return (*left_)(x) * (*right_)(x);  // Произведение значений
}

/**
 * Вычисляет производную произведения по правилу Лейбница
 * Производная произведения: (g*h)' = g'*h + g*h'
 */
double MultiplyFunction::GetDeriv(double x) const {
    // (f*g)' = f'*g + f*g'
    return left_->GetDeriv(x) * (*right_)(x) + (*left_)(x) * right_->GetDeriv(x);
}

/**
 * Строковое представление произведения
 * Формат: "(left * right)"
 */
std::string MultiplyFunction::ToString() const {
    return "(" + left_->ToString() + " * " + right_->ToString() + ")";
}

/**
 * Создает копию функции-произведения
 */
std::shared_ptr<TFunction> MultiplyFunction::Clone() const {
    return std::make_shared<MultiplyFunction>(left_->Clone(), right_->Clone());
}


// DivideFunction implementation
/**
 * Конструктор функции-частного
 * @param left - левая функция (числитель)
 * @param right - правая функция (знаменатель)
 */
DivideFunction::DivideFunction(TFunctionPtr left, TFunctionPtr right) 
    : left_(left), right_(right) {}

/**
 * Вычисляет частное двух функций: f(x) = g(x) / h(x)
 * @throws std::logic_error при делении на ноль
 */
double DivideFunction::operator()(double x) const {
    double denominator = (*right_)(x);  // Вычисляем знаменатель
    if (denominator == 0) {
        throw std::logic_error("Division by zero");  // Проверка деления на ноль
    }
    return (*left_)(x) / denominator;  // Частное значений
}

/**
 * Вычисляет производную частного по правилу дифференцирования дроби
 * Производная частного: (g/h)' = (g'*h - g*h') / h²
 * @throws std::logic_error при делении на ноль в производной
 */
double DivideFunction::GetDeriv(double x) const {
    // (f/g)' = (f'*g - f*g') / g^2
    double f = (*left_)(x);        // Значение числителя
    double g = (*right_)(x);       // Значение знаменателя
    double f_prime = left_->GetDeriv(x);  // Производная числителя
    double g_prime = right_->GetDeriv(x); // Производная знаменателя
    
    if (g == 0) {
        throw std::logic_error("Division by zero in derivative");  // Проверка
    }
    
    return (f_prime * g - f * g_prime) / (g * g);  // Формула производной частного
}

/**
 * Строковое представление частного
 * Формат: "(left / right)"
 */
std::string DivideFunction::ToString() const {
    return "(" + left_->ToString() + " / " + right_->ToString() + ")";
}

/**
 * Создает копию функции-частного
 */
std::shared_ptr<TFunction> DivideFunction::Clone() const {
    return std::make_shared<DivideFunction>(left_->Clone(), right_->Clone());
}


// FunctionFactory implementation
/**
 * Фабричный метод для создания функций
 * Согласно заданию: создает объекты по строковому параметру type
 * @param type - тип функции: "ident", "const", "power", "exp", "polynomial"
 * @param params - параметры функции
 * @return умный указатель на созданную функцию
 * @throws std::logic_error при неизвестном типе или неверных параметрах
 */
TFunctionPtr FunctionFactory::Create(const std::string& type, const std::vector<double>& params) {
    if (type == "ident") {
        // Тождественная функция не требует параметров
        return std::make_shared<IdentFunction>();
    } else if (type == "const") {
        // Константная функция требует один параметр - значение константы
        if (params.size() != 1) {
            throw std::logic_error("Const function requires exactly one parameter");
        }
        return std::make_shared<ConstFunction>(params[0]);
    } else if (type == "power") {
        // Степенная функция требует один параметр - показатель степени
        if (params.size() != 1) {
            throw std::logic_error("Power function requires exactly one parameter");
        }
        return std::make_shared<PowerFunction>(static_cast<int>(params[0]));
    } else if (type == "exp") {
        // Экспоненциальная функция не требует параметров
        return std::make_shared<ExpFunction>();
    } else if (type == "polynomial") {
        // Полиномиальная функция принимает вектор коэффициентов
        return std::make_shared<PolynomialFunction>(params);
    } else {
        // Неизвестный тип функции
        throw std::logic_error("Unknown function type: " + type);
    }
}

/**
 * Перегруженная версия для создания функций с одним double параметром
 */
TFunctionPtr FunctionFactory::Create(const std::string& type, double param) {
    return Create(type, std::vector<double>{param});  // Преобразуем в вектор
}

/**
 * Перегруженная версия для создания функций с одним int параметром
 */
TFunctionPtr FunctionFactory::Create(const std::string& type, int param) {
    return Create(type, std::vector<double>{static_cast<double>(param)});  // Преобразуем int в double
}

// Operator overloads

/**
 * Оператор сложения функций
 * Согласно заданию: результат - TFunction, представляющий композицию через операцию сложения
 */
TFunctionPtr operator+(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<AddFunction>(lhs.Clone(), rhs.Clone());
}

/**
 * Оператор вычитания функций
 */
TFunctionPtr operator-(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<SubtractFunction>(lhs.Clone(), rhs.Clone());
}

/**
 * Оператор умножения функций
 */
TFunctionPtr operator*(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<MultiplyFunction>(lhs.Clone(), rhs.Clone());
}

/**
 * Оператор деления функций
 */
TFunctionPtr operator/(const TFunction& lhs, const TFunction& rhs) {
    return std::make_shared<DivideFunction>(lhs.Clone(), rhs.Clone());
}


// Gradient descent implementation
/**
 * Находит корень уравнения f(x) = 0 методом градиентного спуска
 * Согласно заданию п.4: функция принимает арифметическое выражение f(x)
 * и находит корень уравнения f(x) = 0
 * @param func - функция для поиска корня
 * @param initial_guess - начальное приближение
 * @param iterations - число итераций (проверка сходимости не требуется)
 * @return приближенное значение корня
 * 
 * Алгоритм: x_{n+1} = x_n - learning_rate * f(x_n) / f'(x_n)
 * Это модификация метода Ньютона с фиксированным шагом
 */
double FindRootGradientDescent(const TFunction& func, double initial_guess, int iterations) {
    double x = initial_guess;     // Начальное приближение
    double learning_rate = 0.1;   // Скорость обучения (можно адаптировать)
    
    // Выполняем заданное количество итераций
    for (int i = 0; i < iterations; ++i) {
        double gradient = func.GetDeriv(x);  // Производная в текущей точке
        double value = func(x);              // Значение функции в текущей точке
        
        // Обновляем x по формуле градиентного спуска
        // Добавляем 1e-10 для избежания деления на ноль
        x = x - learning_rate * value / (std::abs(gradient) + 1e-10);
    }
    
    return x;  // Возвращаем найденное приближение
}


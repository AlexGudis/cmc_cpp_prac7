#include <memory>      // Для умных указателей std::shared_ptr
#include <vector>      // (хранение коэффициентов полинома)
#include <string>      
#include <stdexcept>   // Для std::logic_error 
//#include <functional>  // Для std::function (не используется напрямую, но может пригодиться)
#include <iostream>    
#include <cmath>      
#include <sstream>     
//#include <iomanip>     // Для форматирования вывода (не используется напрямую)

// --------------------------------------------------------- Абстрактные методы ------------------------------------------ 

/**
 * Базовый абстрактный класс TFunction - представляет математическую функцию одной переменной
 * Согласно заданию: хранит функторы для вычисления функции и ее производной
 */
class TFunction {
public:
    // Виртуальный деструктор для корректного удаления объектов производных классов
    virtual ~TFunction() = default;
    
    /**
     * Оператор вызова функции - вычисляет значение функции в точке x
     * По заданию: позволяет вычислять значения функции при передаче значения переменной
     */
    virtual double operator()(double x) const = 0;
    
    /**
     * Вычисляет производную функции в точке x
     * По заданию: метод рассчитывающий производную в заданной точке
     * Примечание: производная вычисляется численно через правила дифференцирования, 
     * а не символьное дифференцирование (согласно п.3 задания)
     */
    virtual double GetDeriv(double x) const = 0;
    
    /**
     * Возвращает строковое представление функции
     * По заданию: метод ToString, возвращающий строковое представление функции
     * Примечание: согласно замечанию (1) поддерживается только у основных функций,
     * но для удобства реализован и для составных
     */
    virtual std::string ToString() const = 0;
    
    /**
     * Создает копию объекта (паттерн Prototype)
     * Необходим для корректного создания составных функций через операторы
     */
    virtual std::shared_ptr<TFunction> Clone() const = 0;
};

// Псевдоним для удобства работы с умными указателями на функции
using TFunctionPtr = std::shared_ptr<TFunction>;

// -------------------------------------------------------------------
// Базовые функции (основные функциональные объекты согласно заданию)
// -------------------------------------------------------------------

/**
 * Тождественная функция f(x) = x
 * Согласно заданию: одна из основных функций типа "ident"
 */
class IdentFunction : public TFunction {
public:
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

/**
 * Константная функция f(x) = c (вещественная константа)
 * Согласно заданию: одна из основных функций типа "const"
 */
class ConstFunction : public TFunction {
private:
    double value_;  // Значение константы
public:
    explicit ConstFunction(double value);  // explicit запрещает неявное преобразование
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

/**
 * Степенная функция f(x) = x^n
 * Согласно заданию: одна из основных функций типа "power"
 */
class PowerFunction : public TFunction {
private:
    int power_;  // Показатель степени
public:
    explicit PowerFunction(int power);
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

/**
 * Экспоненциальная функция f(x) = exp(x)
 * Согласно заданию: одна из основных функций типа "exp"
 */
class ExpFunction : public TFunction {
public:
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

/**
 * Полиномиальная функция f(x) = a₀ + a₁x + a₂x² + ... + aₙxⁿ
 * Согласно заданию: одна из основных функций типа "polynomial"
 * Коэффициенты хранятся в векторе: coefficients_[0] = a₀, coefficients_[1] = a₁, и т.д.
 */
class PolynomialFunction : public TFunction {
private:
    std::vector<double> coefficients_;  // Коэффициенты полинома
public:
    explicit PolynomialFunction(const std::vector<double>& coefficients);
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

// -------------------------------------------------------------------
// Составные функции (результаты арифметических операций)
// Согласно заданию п.2: результат применения оператора - TFunction,
// представляющий "композицию" функций посредством арифметической операции
// -------------------------------------------------------------------

/**
 * Функция-сумма: f(x) = g(x) + h(x)
 * Реализует оператор сложения "+"
 */
class AddFunction : public TFunction {
private:
    TFunctionPtr left_, right_;  // Левый и правый операнды
public:
    AddFunction(TFunctionPtr left, TFunctionPtr right);
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

/**
 * Функция-разность: f(x) = g(x) - h(x)
 * Реализует оператор вычитания "-"
 */
class SubtractFunction : public TFunction {
private:
    TFunctionPtr left_, right_;  // Левый и правый операнды
public:
    SubtractFunction(TFunctionPtr left, TFunctionPtr right);
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

/**
 * Функция-произведение: f(x) = g(x) * h(x)
 * Реализует оператор умножения "*"
 */
class MultiplyFunction : public TFunction {
private:
    TFunctionPtr left_, right_;  // Левый и правый операнды
public:
    MultiplyFunction(TFunctionPtr left, TFunctionPtr right);
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

/**
 * Функция-частное: f(x) = g(x) / h(x)
 * Реализует оператор деления "/"
 * Генерирует исключение при делении на ноль
 */
class DivideFunction : public TFunction {
private:
    TFunctionPtr left_, right_;  // Левый и правый операнды
public:
    DivideFunction(TFunctionPtr left, TFunctionPtr right);
    double operator()(double x) const override;
    double GetDeriv(double x) const override;
    std::string ToString() const override;
    std::shared_ptr<TFunction> Clone() const override;
};

// -------------------------------------------------------------------
// Фабрика функций (паттерн Factory Method)
// Согласно заданию п.1: фабрика по созданию основных функциональных объектов
// -------------------------------------------------------------------
class FunctionFactory {
public:
    /**
     * Основной метод создания функций
     * @param type - тип функции из множества {"ident", "const", "power", "exp", "polynomial"}
     * @param params - параметры для создания функции (значение константы, степень, коэффициенты)
     * @return умный указатель на созданную функцию
     * @throws std::logic_error при неподдерживаемом типе или неверных параметрах
     */
    static TFunctionPtr Create(const std::string& type, const std::vector<double>& params = {});
    
    // Перегруженные версии для удобства
    static TFunctionPtr Create(const std::string& type, double param);
    static TFunctionPtr Create(const std::string& type, int param);
};

// -------------------------------------------------------------------
// Перегрузка операторов
// Согласно заданию п.2: поддержка арифметических выражений через переопределение операторов
// -------------------------------------------------------------------
TFunctionPtr operator+(const TFunction& lhs, const TFunction& rhs);
TFunctionPtr operator-(const TFunction& lhs, const TFunction& rhs);
TFunctionPtr operator*(const TFunction& lhs, const TFunction& rhs);
TFunctionPtr operator/(const TFunction& lhs, const TFunction& rhs);

// -------------------------------------------------------------------
// Утилиты
// -------------------------------------------------------------------

/**
 * Находит корень уравнения f(x) = 0 методом градиентного спуска
 * Согласно заданию п.4: функция принимает арифметическое выражение f(x) 
 * и находит корень уравнения f(x) = 0 методом градиентного спуска
 * @param func - функция для поиска корня
 * @param initial_guess - начальное приближение
 * @param iterations - число итераций (согласно заданию, проверка сходимости не требуется)
 * @return приближенное значение корня
 */
double FindRootGradientDescent(const TFunction& func, double initial_guess, int iterations);




// -------------------------------------------- Реализация --------------------------------------------

// IdentFunction implementation

/**
 * Вычисляет значение тождественной функции f(x) = x
 */
double IdentFunction::operator()(double x) const {
    return x;  // Просто возвращаем переданный аргумент
}

/**
 * Вычисляет производную тождественной функции
 * Производная f(x) = x равна f'(x) = 1
 */
double IdentFunction::GetDeriv(double x) const {
    return 1.0;  // Производная константна
}

/**
 * Строковое представление тождественной функции
 */
std::string IdentFunction::ToString() const {
    return "x";  // Просто "x"
}

/**
 * Создает копию тождественной функции
 */
std::shared_ptr<TFunction> IdentFunction::Clone() const {
    return std::make_shared<IdentFunction>();  // Создаем новый объект
}

// ConstFunction implementation

/**
 * Конструктор константной функции
 * @param value - значение константы
 */
ConstFunction::ConstFunction(double value) : value_(value) {}

/**
 * Вычисляет значение константной функции f(x) = c
 * Всегда возвращает значение константы, независимо от x
 */
double ConstFunction::operator()(double x) const {
    return value_;  // Игнорируем x, возвращаем константу
}

/**
 * Вычисляет производную константной функции
 * Производная константы всегда равна 0
 */
double ConstFunction::GetDeriv(double x) const {
    return 0.0;  // Производная константы равна нулю
}

/**
 * Строковое представление константной функции
 * Преобразует числовое значение в строку
 */
std::string ConstFunction::ToString() const {
    std::ostringstream oss;  // Поток для построения строки
    oss << value_;           // Записываем значение в поток
    return oss.str();        // Возвращаем полученную строку
}

/**
 * Создает копию константной функции
 */
std::shared_ptr<TFunction> ConstFunction::Clone() const {
    return std::make_shared<ConstFunction>(value_);  // Создаем копию с тем же значением
}

// PowerFunction implementation

/**
 * Конструктор степенной функции: x**power
 * @param power - показатель степени
 */
PowerFunction::PowerFunction(int power) : power_(power) {}

/**
 * Вычисляет значение степенной функции f(x) = x^n
 * Использует std::pow для возведения в степень
 */
double PowerFunction::operator()(double x) const {
    return std::pow(x, power_);  // x в степени power_
}

/**
 * Вычисляет производную степенной функции
 * Производная f(x) = x^n равна f'(x) = n*x^(n-1)
 * Для n=0 (константа) производная равна 0
 */
double PowerFunction::GetDeriv(double x) const {
    if (power_ == 0) return 0.0;  // Производная константы
    return power_ * std::pow(x, power_ - 1);  // n*x^(n-1)
}

/**
 * Строковое представление степенной функции
 * Специальные случаи для степени 0 и 1
 */
std::string PowerFunction::ToString() const {
    if (power_ == 0) return "1";      // x^0 = 1
    if (power_ == 1) return "x";      // x^1 = x
    return "x^" + std::to_string(power_);  // x^n для n>1
}

/**
 * Создает копию степенной функции
 */
std::shared_ptr<TFunction> PowerFunction::Clone() const {
    return std::make_shared<PowerFunction>(power_);
}

// ExpFunction implementation

/**
 * Вычисляет значение экспоненциальной функции f(x) = exp(x)
 */
double ExpFunction::operator()(double x) const {
    return std::exp(x);  // e в степени x
}

/**
 * Вычисляет производную экспоненциальной функции
 * Производная exp(x) равна exp(x)
 */
double ExpFunction::GetDeriv(double x) const {
    return std::exp(x);  // Производная экспоненты равна самой экспоненте
}

/**
 * Строковое представление экспоненциальной функции
 */
std::string ExpFunction::ToString() const {
    return "exp(x)";
}

/**
 * Создает копию экспоненциальной функции
 */
std::shared_ptr<TFunction> ExpFunction::Clone() const {
    return std::make_shared<ExpFunction>();
}

// PolynomialFunction implementation

/**
 * Конструктор полиномиальной функции
 * @param coefficients - вектор коэффициентов: [a₀, a₁, a₂, ..., aₙ]
 * где полином: a₀ + a₁x + a₂x² + ... + aₙxⁿ
 */
PolynomialFunction::PolynomialFunction(const std::vector<double>& coefficients) 
    : coefficients_(coefficients) {}  // Инициализируем коэффициенты

/**
 * Вычисляет значение полинома в точке x using Horner's method
 * Полином: a₀ + a₁x + a₂x² + ... + aₙxⁿ
 */
double PolynomialFunction::operator()(double x) const {
    double result = 0.0;    // Накапливаем результат
    double x_power = 1.0;   // Текущая степень x (начинаем с x^0 = 1)
    
    // Проходим по всем коэффициентам
    for (double coef : coefficients_) {
        result += coef * x_power;  // Добавляем aᵢ * xⁱ
        x_power *= x;              // Увеличиваем степень для следующего коэффициента
    }
    
    return result;
}

/**
 * Вычисляет производную полинома в точке x
 * Производная: a₁ + 2a₂x + 3a₃x² + ... + naₙxⁿ⁻¹
 * Для полинома степени 0 (константа) производная равна 0
 */
double PolynomialFunction::GetDeriv(double x) const {
    if (coefficients_.size() <= 1) return 0.0;  // Производная константы или пустого полинома
    
    double result = 0.0;
    double x_power = 1.0;  // Начинаем с x^0
    
    // Начинаем с i=1, так как производная от a₀ равна 0
    for (size_t i = 1; i < coefficients_.size(); ++i) {
        result += coefficients_[i] * i * x_power;  // Добавляем i*aᵢ * xⁱ⁻¹
        x_power *= x;                              // Увеличиваем степень
    }
    
    return result;
}

/**
 * Строковое представление полинома
 * Форматирует полином в читаемом виде: "1 + 2*x - 3*x^2"
 */
std::string PolynomialFunction::ToString() const {
    if (coefficients_.empty()) return "0";  // Пустой полином
    
    std::ostringstream oss;  // Поток для построения строки
    bool first_term = true;  // Флаг первого слагаемого (для знака)
    
    // Проходим по всем коэффициентам
    for (size_t i = 0; i < coefficients_.size(); ++i) {
        if (coefficients_[i] == 0) continue;  // Пропускаем нулевые коэффициенты
        
        // Обрабатываем знак
        if (!first_term) {
            // Не первый член - добавляем знак
            oss << (coefficients_[i] > 0 ? " + " : " - ");
        } else if (coefficients_[i] < 0) {
            // Первый член с отрицательным знаком
            oss << "-";
        }
        
        double abs_coef = std::abs(coefficients_[i]);  // Абсолютное значение коэффициента
        
        // Форматируем член в зависимости от степени
        if (i == 0) {
            // Свободный член: просто число
            oss << abs_coef;
        } else {
            // Член с x
            if (abs_coef != 1.0) {
                // Коэффициент не равен 1 - показываем его
                oss << abs_coef << "*";
            }
            oss << "x";
            if (i > 1) {
                // Степень больше 1 - показываем степень
                oss << "^" << i;
            }
        }
        
        first_term = false;  // Больше не первый член
    }
    
    return oss.str();
}

/**
 * Создает копию полиномиальной функции
 */
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
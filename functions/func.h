#ifndef FUNCTION_LIBRARY_H
#define FUNCTION_LIBRARY_H

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

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
    
#endif
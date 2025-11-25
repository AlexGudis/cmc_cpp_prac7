#ifndef FUNCTION_LIBRARY_H
#define FUNCTION_LIBRARY_H

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

// --------------------------------------------------------- Абстрактные методы ------------------------------------------ 


// Базовый абстрактный класс TFunction - представляет математическую функцию одной переменной
class TFunction {
    public:
        virtual ~TFunction() = default;

        // Оператор вызова функции - вычисляет значение функции в точке x
        virtual double operator()(double x) const = 0;
        
        // Вычисляет производную функции в точке x
        virtual double GetDeriv(double x) const = 0;
        
        // Возвращает строковое представление функции
        virtual std::string ToString() const = 0;
        
        /**
         * Создает копию объекта (паттерн Prototype)
         * Необходим для корректного создания составных функций через операторы
         */
        virtual std::shared_ptr<TFunction> Clone() const = 0;
    };
    
    using TFunctionPtr = std::shared_ptr<TFunction>;
    


    // ------------------------------------------------------------------- Базовые функции ------------------------------------------------------------------- 
    // Тождественная функция f(x) = x
    class IdentFunction : public TFunction {
    public:
        double operator()(double x) const override;
        double GetDeriv(double x) const override;
        std::string ToString() const override;
        std::shared_ptr<TFunction> Clone() const override;
    };
    
    // Константная функция f(x) = c
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
    
    // Степенная функция f(x) = x^n
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
    
    // Экспоненциальная функция f(x) = exp(x)
    class ExpFunction : public TFunction {
    public:
        double operator()(double x) const override;
        double GetDeriv(double x) const override;
        std::string ToString() const override;
        std::shared_ptr<TFunction> Clone() const override;
    };
    
    // Полиномиальная функция f(x) = a_0 + a_1 * x + a_2 * x^2 + ... + a_n * x^n
    class PolynomialFunction : public TFunction {
    private:
        std::vector<double> coefficients_;  // a_0, a_1, ...
    public:
        explicit PolynomialFunction(const std::vector<double>& coefficients);
        double operator()(double x) const override;
        double GetDeriv(double x) const override;
        std::string ToString() const override;
        std::shared_ptr<TFunction> Clone() const override;
    };
    


    // -----------------------------  Составные функции (арифм. операции) ---------------------------------------
    // Функция-сумма: f(x) = g(x) + h(x)
    class AddFunction : public TFunction {
    private:
        TFunctionPtr left_, right_;
    public:
        AddFunction(TFunctionPtr left, TFunctionPtr right);
        double operator()(double x) const override;
        double GetDeriv(double x) const override;
        std::string ToString() const override;
        std::shared_ptr<TFunction> Clone() const override;
    };
    
    // Функция-разность: f(x) = g(x) - h(x)
    class SubtractFunction : public TFunction {
    private:
        TFunctionPtr left_, right_;
    public:
        SubtractFunction(TFunctionPtr left, TFunctionPtr right);
        double operator()(double x) const override;
        double GetDeriv(double x) const override;
        std::string ToString() const override;
        std::shared_ptr<TFunction> Clone() const override;
    };
    
    // Функция-произведение: f(x) = g(x) * h(x)
    class MultiplyFunction : public TFunction {
    private:
        TFunctionPtr left_, right_;
    public:
        MultiplyFunction(TFunctionPtr left, TFunctionPtr right);
        double operator()(double x) const override;
        double GetDeriv(double x) const override;
        std::string ToString() const override;
        std::shared_ptr<TFunction> Clone() const override;
    };
    
    // Функция-частное: f(x) = g(x) / h(x)
    class DivideFunction : public TFunction {
    private:
        TFunctionPtr left_, right_;
    public:
        DivideFunction(TFunctionPtr left, TFunctionPtr right);
        double operator()(double x) const override;
        double GetDeriv(double x) const override;
        std::string ToString() const override;
        std::shared_ptr<TFunction> Clone() const override;
    };
    
    // ------------------------------------------ Фабрика ------------------------------------------
    class FunctionFactory {
    public:
        /**
         * Основной метод создания функций
         * type - тип функции
         * params - параметры для создания функции
         */
        static TFunctionPtr Create(const std::string& type, const std::vector<double>& params = {});
        
        // Перегруженные версии для удобства
        static TFunctionPtr Create(const std::string& type, double param);
        static TFunctionPtr Create(const std::string& type, int param);
    };
    
    // Перегрузка операторов
    TFunctionPtr operator+(const TFunction& lhs, const TFunction& rhs);
    TFunctionPtr operator-(const TFunction& lhs, const TFunction& rhs);
    TFunctionPtr operator*(const TFunction& lhs, const TFunction& rhs);
    TFunctionPtr operator/(const TFunction& lhs, const TFunction& rhs);
    

    /**
     * Находит корень уравнения f(x) = 0 методом градиентного спуска
     * func - функция для поиска корня
     * initial_guess - начальное приближение
     * iterations - число итераций
     */
    double FindRootGradientDescent(const TFunction& func, double initial_guess, int iterations);
    
#endif
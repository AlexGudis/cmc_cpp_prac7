#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <cmath>
#include <functional>
#include <chrono>

// Абстрактный класс для представления решения
class Solution {
public:
    virtual ~Solution() = default;
    virtual double evaluate() const = 0; // Функция оценки качества решения
    virtual std::unique_ptr<Solution> clone() const = 0; // Клонирование решения
    virtual void print() const = 0; // Для отладки
};

// Абстрактный класс для операции изменения решения
class MutationOperator {
public:
    virtual ~MutationOperator() = default;
    virtual std::unique_ptr<Solution> apply(const Solution& solution) = 0; // Применение мутации
};

// Абстрактный класс для закона понижения температуры
class CoolingSchedule {
public:
    virtual ~CoolingSchedule() = default;
    virtual double getTemperature(int iteration) = 0; // Получение температуры для итерации
};

// Закон Больцмана: T = T0 / ln(1 + i)
class BoltzmannCooling : public CoolingSchedule {
private:
    double T0_;
public:
    BoltzmannCooling(double T0) : T0_(T0) {}
    
    double getTemperature(int iteration) override {
        if (iteration == 0) return T0_;
        return T0_ / std::log(1 + iteration);
    }
};

// Закон Коши: T = T0 / (1 + i)
class CauchyCooling : public CoolingSchedule {
private:
    double T0_;
public:
    CauchyCooling(double T0) : T0_(T0) {}
    
    double getTemperature(int iteration) override {
        return T0_ / (1 + iteration);
    }
};

// Смешанный закон: T = T0 * ln(1 + i) / (1 + i)
class MixedCooling : public CoolingSchedule {
private:
    double T0_;
public:
    MixedCooling(double T0) : T0_(T0) {}
    
    double getTemperature(int iteration) override {
        if (iteration == 0) return T0_;
        return T0_ * std::log(1 + iteration) / (1 + iteration);
    }
};

// Головной класс алгоритма имитации отжига
class SimulatedAnnealing {
private:
    std::unique_ptr<Solution> currentSolution_;
    std::unique_ptr<Solution> bestSolution_;
    std::unique_ptr<MutationOperator> mutationOperator_;
    std::unique_ptr<CoolingSchedule> coolingSchedule_;
    double initialTemperature_;
    int iterationsPerTemperature_;
    int maxIterationsWithoutImprovement_;
    
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
    
public:
    SimulatedAnnealing(
        std::unique_ptr<Solution> initialSolution,
        std::unique_ptr<MutationOperator> mutationOp,
        std::unique_ptr<CoolingSchedule> cooling,
        double initialTemp,
        int iterPerTemp,
        int maxIterWithoutImprovement
    ) : currentSolution_(std::move(initialSolution)),
        mutationOperator_(std::move(mutationOp)),
        coolingSchedule_(std::move(cooling)),
        initialTemperature_(initialTemp),
        iterationsPerTemperature_(iterPerTemp),
        maxIterationsWithoutImprovement_(maxIterWithoutImprovement)
    {
        // Инициализация генератора случайных чисел
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        rng_.seed(seed);
        dist_ = std::uniform_real_distribution<double>(0.0, 1.0);
        
        bestSolution_ = currentSolution_->clone();
    }
    
    void run() {
        int iteration = 0;
        int iterationsWithoutImprovement = 0;
        double currentTemperature = initialTemperature_;
        
        std::cout << "Starting Simulated Annealing..." << std::endl;
        std::cout << "Initial solution quality: " << bestSolution_->evaluate() << std::endl;
        
        while (iterationsWithoutImprovement < maxIterationsWithoutImprovement_) {
            // Шаг 3-5: Итерации при текущей температуре
            for (int i = 0; i < iterationsPerTemperature_; i++) {
                // Шаг 3: Применение операции преобразования
                auto newSolution = mutationOperator_->apply(*currentSolution_);
                
                // Шаг 4: Вычисление изменения функции оценки
                double deltaF = newSolution->evaluate() - currentSolution_->evaluate();
                
                // Принятие решения о переходе к новому решению
                if (deltaF <= 0) {
                    // Решение улучшилось
                    currentSolution_ = std::move(newSolution);
                    
                    // Проверка, является ли это решение лучшим
                    if (currentSolution_->evaluate() < bestSolution_->evaluate()) {
                        bestSolution_ = currentSolution_->clone();
                        iterationsWithoutImprovement = 0; // Сброс счетчика
                        std::cout << "New best solution found: " << bestSolution_->evaluate() 
                                  << " at iteration " << iteration << std::endl;
                    }
                } else {
                    // Решение ухудшилось - принимаем с вероятностью exp(-ΔF/T)
                    double probability = std::exp(-deltaF / currentTemperature);
                    if (dist_(rng_) < probability) {
                        currentSolution_ = std::move(newSolution);
                    }
                }
                
                iteration++;
            }
            
            // Шаг 6: Проверка критерия останова (уже в условии цикла)
            
            // Шаг 7: Понижение температуры
            currentTemperature = coolingSchedule_->getTemperature(iteration);
            
            iterationsWithoutImprovement++;
        }
        
        std::cout << "Optimization completed." << std::endl;
        std::cout << "Best solution quality: " << bestSolution_->evaluate() << std::endl;
        std::cout << "Best solution: ";
        bestSolution_->print();
    }
    
    std::unique_ptr<Solution> getBestSolution() {
        return bestSolution_->clone();
    }
};

// Пример конкретной реализации: задача минимизации функции Rastrigin
class RastriginSolution : public Solution {
private:
    std::vector<double> coordinates_;
    int dimensions_;
    
public:
    RastriginSolution(int dim, double minVal = -5.12, double maxVal = 5.12) : dimensions_(dim) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(minVal, maxVal);
        
        coordinates_.resize(dimensions_);
        for (int i = 0; i < dimensions_; i++) {
            coordinates_[i] = dist(gen);
        }
    }
    
    RastriginSolution(const std::vector<double>& coords) : coordinates_(coords), dimensions_(coords.size()) {}
    
    double evaluate() const override {
        // Функция Rastrigin: f(x) = A*n + Σ[x_i² - A*cos(2πx_i)], где A=10
        const double A = 10.0;
        double sum = A * dimensions_;
        
        for (double x : coordinates_) {
            sum += x * x - A * std::cos(2 * M_PI * x);
        }
        
        return sum;
    }
    
    std::unique_ptr<Solution> clone() const override {
        return std::make_unique<RastriginSolution>(coordinates_);
    }
    
    void print() const override {
        std::cout << "[";
        for (size_t i = 0; i < coordinates_.size(); i++) {
            std::cout << coordinates_[i];
            if (i < coordinates_.size() - 1) std::cout << ", ";
        }
        std::cout << "]";
    }
    
    const std::vector<double>& getCoordinates() const { return coordinates_; }
    void setCoordinates(const std::vector<double>& coords) { coordinates_ = coords; }
};

// Мутация для решения Rastrigin
class RastriginMutation : public MutationOperator {
private:
    double mutationStrength_;
    
public:
    RastriginMutation(double strength = 0.1) : mutationStrength_(strength) {}
    
    std::unique_ptr<Solution> apply(const Solution& solution) override {
        const RastriginSolution& rastriginSol = static_cast<const RastriginSolution&>(solution);
        auto newSolution = rastriginSol.clone();
        RastriginSolution* newRastrigin = static_cast<RastriginSolution*>(newSolution.get());
        
        std::vector<double> newCoords = newRastrigin->getCoordinates();
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> dist(0.0, mutationStrength_);
        
        for (double& coord : newCoords) {
            coord += dist(gen);
            // Ограничение координат в диапазоне [-5.12, 5.12]
            if (coord < -5.12) coord = -5.12;
            if (coord > 5.12) coord = 5.12;
        }
        
        newRastrigin->setCoordinates(newCoords);
        return newSolution;
    }
};

// Демонстрация использования
int main() {
    // Параметры алгоритма
    const double INITIAL_TEMPERATURE = 100.0;
    const int ITERATIONS_PER_TEMP = 50;
    const int MAX_ITER_WITHOUT_IMPROVEMENT = 100;
    const int DIMENSIONS = 5;
    
    // Создание начального решения
    auto initialSolution = std::make_unique<RastriginSolution>(DIMENSIONS);
    
    // Создание оператора мутации
    auto mutationOp = std::make_unique<RastriginMutation>(0.5);
    
    // Выбор закона охлаждения (можно менять)
    auto coolingSchedule = std::make_unique<BoltzmannCooling>(INITIAL_TEMPERATURE);
    // auto coolingSchedule = std::make_unique<CauchyCooling>(INITIAL_TEMPERATURE);
    // auto coolingSchedule = std::make_unique<MixedCooling>(INITIAL_TEMPERATURE);
    
    // Создание и запуск алгоритма
    SimulatedAnnealing sa(
        std::move(initialSolution),
        std::move(mutationOp),
        std::move(coolingSchedule),
        INITIAL_TEMPERATURE,
        ITERATIONS_PER_TEMP,
        MAX_ITER_WITHOUT_IMPROVEMENT
    );
    
    sa.run();
    
    return 0;
}
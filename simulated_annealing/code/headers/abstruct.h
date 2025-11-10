using namespace std;
// ------------------------------ Общие абстрактные интерфейсы ------------------------------
struct Solution {
    virtual ~Solution() = default;
    // значение минимизируемого критерия (энергия). Чем меньше — лучше.
    virtual double criteria() const = 0;
    // глубокая копия решения
    virtual unique_ptr<Solution> clone() const = 0;
    // текстовое представление (для печати)
    virtual string toString() const = 0;
};

struct Mutation {
    virtual ~Mutation() = default;
    // применить мутацию к решению (изменяет решение in-place)
    // Бросает bad_cast если тип решения не тот, который ожидает мутация.
    virtual void apply(Solution &s, std::mt19937 &rng) const = 0;
};

struct CoolingLaw {
    virtual ~CoolingLaw() = default;
    // возвращает новую температуру на следующей итерации
    // (можно использовать номер итерации, текущую температуру и т.п.)
    virtual double nextTemperature(double currentT, int iter) = 0;
    // virtual string name() const = 0;
};
/*
main2.cpp
Симуляция имитации отжига для задачи расписания N работ на M процессорах
Компиляция: g++ -std=c++17 main2.cpp -O2 -o main2

Варианты запуска кода:
1) Данные генерируются автоматически и/или указываются в коде
2) Данные N, M, закон понижения температуры берутся из параметров и работы генерируются
3) Данные N, M, закон понижения температуры и работы вводятся пользователем
4) Данные N, M, закон понижения температуры и работы берутся из файла
*/

#include <bits/stdc++.h>
using namespace std;

/*
  Архитектура:
   - class Solution (abstract): интерфейс для представления решения и вычисления "энергии" (критерия).
   - class Mutation (abstract): абстракция операции мутации (изменения решения).
   - class CoolingLaw (abstract): интерфейс закона понижения температуры.
   - class SimulatedAnnealing: главный класс, реализующий цикл ИО, принимает конкретные реализации выше.
   - Конкретная реализация для задачи расписания: ScheduleSolution, конкретные мутации (Swap, Move).
   - Реализованы три закона понижения температуры: Exponential, Linear, Logarithmic.
   - Генератор данных и пример запуска в main().
*/

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

// ------------------------------ Реализация задачи расписания ------------------------------
struct ScheduleSolution : Solution {
    // Представление: список работ (0..N-1) распределён по M процессорам,
    // на каждом процессоре порядок выполнения задан вектором jobLists[j].
    int N; // число работ
    int M; // число процессоров
    vector<int> w; // длительности работ w[i], расположены по индексам. По индексу работы получаю её время
    vector<vector<int>> jobLists; // jobLists[j] - список индексов работ на процессоре j

    // Кеш сумм по процессорам (для быстрого пересчёта) — обновляется по необходимости
    mutable vector<int> procSumCache;
    mutable bool cacheValid = false;

    ScheduleSolution() = default;

    ScheduleSolution(int N_, int M_, const vector<int> &w_) : N(N_), M(M_), w(w_) {
        jobLists.assign(M, {});
        // по умолчанию стартовая случайная раздача: round-robin
        for (int i = 0; i < N; ++i) {
            jobLists[i % M].push_back(i);
        }
        procSumCache.assign(M, 0);
        cacheValid = false;
    }

    // создать случайное начальное решение (равномерное распределение)
    static unique_ptr<ScheduleSolution> randomInit(int N, int M, const vector<int>& w, mt19937 &rng) {
        auto s = make_unique<ScheduleSolution>(N, M, w);
        // случайная перестановка работ, затем разложить по очереди
        vector<int> perm(N);
        iota(perm.begin(), perm.end(), 0);
        shuffle(perm.begin(), perm.end(), rng);
        s->jobLists.assign(M, {});
        for (int i = 0; i < N; ++i) {
            s->jobLists[i % M].push_back(perm[i]);
        }
        s->cacheValid = false;
        return s;
    }

    // глубокая копия
    unique_ptr<Solution> clone() const override {
        auto s = make_unique<ScheduleSolution>();
        s->N = N; s->M = M; s->w = w;
        s->jobLists = jobLists;
        s->cacheValid = cacheValid;
        s->procSumCache = procSumCache;
        return s;
    }

    // вычислить суммарное время на каждом процессоре и кэшировать
    void computeProcSums() const {
        if (cacheValid) return;
        procSumCache.assign(M, 0);
        for (int j = 0; j < M; ++j) {
            int sum = 0;
            for (int id : jobLists[j]) sum += w[id];
            procSumCache[j] = sum;
        }
        cacheValid = true;
    }

    // K1 = T_max - T_min (разбалансированность по суммарному времени каждого процессора)
    // double energy() const override {
    //     computeProcSums();
    //     if (M == 0) return 0.0;
    //     int Tmax = *max_element(procSumCache.begin(), procSumCache.end());
    //     int Tmin = *min_element(procSumCache.begin(), procSumCache.end());
    //     return double(Tmax - Tmin);
    // }


    // Вычисление значения целевой функции. В нашем случае это критерий K1, который стараемся минимизировать
    double criteria() const override {
        // Массив всех времен завершения всех работ
        vector<int> finishTimes;
    
        // Идём по всем процессорам
        for (int j = 0; j < M; ++j) {
            int currentTime = 0;

            // Идём по работам на этом процессоре
            for (int job : jobLists[j]) {
                currentTime += w[job];      // прибавляем длительность работы
                finishTimes.push_back(currentTime); // фиксируем момент её завершения
            }
        }
    
        if (finishTimes.empty()) return 0.0;
    
        int Tmax = *max_element(finishTimes.begin(), finishTimes.end());
        int Tmin = *min_element(finishTimes.begin(), finishTimes.end());
    
        return double(Tmax - Tmin);
    }

    // Возвращает Tmax, Tmin и вектор сумм (полезно для вывода)
    tuple<int,int,vector<int>> getProcStats() const {
        computeProcSums();
        int Tmax = *max_element(procSumCache.begin(), procSumCache.end());
        int Tmin = *min_element(procSumCache.begin(), procSumCache.end());
        return {Tmax, Tmin, procSumCache};
    }

    string toString() const override {
        ostringstream oss;

        // TODO: избавиться от getProcStats и прочего кеша, это лишнее. Вывод мне сейчас нравится
        auto [Tmax, Tmin, sums] = getProcStats();
        oss << "Schedule (M=" << M << ", N=" << N << "): (K1)=" << criteria() << "\n";
        for (int j = 0; j < M; ++j) {
            oss << " CPU " << j << " (sum=" << sums[j] << "): ";
            for (size_t k = 0; k < jobLists[j].size(); ++k) {
                int id = jobLists[j][k];
                oss << id << "(" << w[id] << ")";
                if (k + 1 < jobLists[j].size()) oss << ",";
            }
            oss << "\n";
        }
        return oss.str();
    }

    // утилиты: переместить работу из (p_from, idx_in_from) в (p_to, pos)
    void moveJob(int p_from, int idx_in_from, int p_to, int pos) {
        if (p_from < 0 || p_from >= M || p_to < 0 || p_to >= M) return;
        if (idx_in_from < 0 || idx_in_from >= (int)jobLists[p_from].size()) return;
        int job = jobLists[p_from][idx_in_from];
        jobLists[p_from].erase(jobLists[p_from].begin() + idx_in_from);
        if (pos < 0) pos = 0;
        if (pos > (int)jobLists[p_to].size()) pos = jobLists[p_to].size();
        jobLists[p_to].insert(jobLists[p_to].begin() + pos, job);
        cacheValid = false;
    }

    // swap двух работ (p1,i1) и (p2,i2)
    void swapJobs(int p1, int i1, int p2, int i2) {
        if (p1 < 0 || p1 >= M || p2 < 0 || p2 >= M) return;
        if (i1 < 0 || i1 >= (int)jobLists[p1].size()) return;
        if (i2 < 0 || i2 >= (int)jobLists[p2].size()) return;
        std::swap(jobLists[p1][i1], jobLists[p2][i2]);
        cacheValid = false;
    }
};


// ------------------------------ Конкретные мутации ------------------------------

// 1) SwapTwoJobs: выбирает случайно две работы (возможно на одном и том же CPU) и меняет их местами.
struct SwapTwoJobs : Mutation {
    void apply(Solution &s, mt19937 &rng) const override {
        auto *sch = dynamic_cast<ScheduleSolution*>(&s);
        if (!sch) throw bad_cast();
        uniform_int_distribution<int> cpuDist(0, max(0, sch->M - 1));
        // выбираем два CPU (возможно равные)
        int p1 = cpuDist(rng);
        int p2 = cpuDist(rng);
        if (sch->jobLists[p1].empty()) {
            // если пустой, попробуем найти непустой
            for (int i = 0; i < sch->M; ++i) if (!sch->jobLists[i].empty()) { p1 = i; break; }
        }
        if (sch->jobLists[p2].empty()) {
            for (int i = 0; i < sch->M; ++i) if (!sch->jobLists[i].empty()) { p2 = i; break; }
        }
        if (sch->jobLists[p1].empty() || sch->jobLists[p2].empty()) return; // некуда swap'ить
        uniform_int_distribution<int> idx1(0, sch->jobLists[p1].size() - 1);
        uniform_int_distribution<int> idx2(0, sch->jobLists[p2].size() - 1);
        int i1 = idx1(rng);
        int i2 = idx2(rng);
        sch->swapJobs(p1, i1, p2, i2);
    }
};

// 2) MoveJob: взять случайную работу и переместить её в случайную позицию на другом процессоре (или в другой позиции того же).
struct MoveJob : Mutation {
    void apply(Solution &s, mt19937 &rng) const override {
        auto *sch = dynamic_cast<ScheduleSolution*>(&s);
        if (!sch) throw bad_cast();
        // найти непустой процессор-источник
        vector<int> nonEmpty;
        for (int j = 0; j < sch->M; ++j) if (!sch->jobLists[j].empty()) nonEmpty.push_back(j);
        if (nonEmpty.empty()) return;
        uniform_int_distribution<int> pickSrc(0, nonEmpty.size() - 1);
        int p_from = nonEmpty[pickSrc(rng)];
        uniform_int_distribution<int> idxFrom(0, sch->jobLists[p_from].size() - 1);
        int idx_in_from = idxFrom(rng);

        uniform_int_distribution<int> pickCpu(0, sch->M - 1);
        int p_to = pickCpu(rng);
        uniform_int_distribution<int> pos(0, max(0, (int)sch->jobLists[p_to].size()));
        int position = pos(rng);

        sch->moveJob(p_from, idx_in_from, p_to, position);
    }
};

// Можно добавить смесь мутаций: случайный выбор одного из наборов
struct CompositeMutation : Mutation {
    vector<shared_ptr<Mutation>> muts;
    CompositeMutation(const vector<shared_ptr<Mutation>>& v) : muts(v) {}
    void apply(Solution &s, mt19937 &rng) const override {
        uniform_int_distribution<int> dist(0, (int)muts.size()-1);
        muts[dist(rng)]->apply(s, rng);
    }
};

// ------------------------------ Законы понижения температуры ------------------------------
// Закон Больцмана: T = T0 / ln(1 + i)
class BoltzmannCooling : public CoolingLaw {
private:
    double T0_;
public:
    BoltzmannCooling(double T0) : T0_(T0) {}
    
    double nextTemperature(double currentT, int iter) override {
        if (iter == 0) return T0_;
        return T0_ / std::log(1 + iter);
    }
};

// Закон Коши: T = T0 / (1 + i)
class CauchyCooling : public CoolingLaw {
private:
    double T0_;
public:
    CauchyCooling(double T0) : T0_(T0) {}
    
    double nextTemperature(double currentT, int iter) override {
        return T0_ / (1 + iter);
    }
};

// Смешанный закон: T = T0 * ln(1 + i) / (1 + i)
class MixedCooling : public CoolingLaw {
private:
    double T0_;
public:
    MixedCooling(double T0) : T0_(T0) {}
    
    double nextTemperature(double currentT, int iter) override {
        if (iter == 0) return T0_;
        return T0_ * std::log(1 + iter) / (1 + iter);
    }
};


// ------------------------------ Главный класс ИО ------------------------------
struct SimulatedAnnealing {
    // параметры
    double T0;
    double Tmin;
    int maxIterations; // макс. число итераций (внешних шагов)
    int noImproveLimit; // число итераций без улучшения для остановки (K = 100 по ТЗ)
    unique_ptr<CoolingLaw> cooling;
    shared_ptr<Mutation> mutation;
    mt19937 rng;

    SimulatedAnnealing(double T0_, double Tmin_, int maxIter_, int noImproveLimit_,
                       unique_ptr<CoolingLaw> cooling_, shared_ptr<Mutation> mutation_, uint32_t seed = 0)
        : T0(T0_), Tmin(Tmin_), maxIterations(maxIter_), noImproveLimit(noImproveLimit_),
          cooling(move(cooling_)), mutation(mutation_)
    {
        if (seed == 0) {
            random_device rd;
            seed = rd();
        }
        rng.seed(seed);
    }

    // Запуск ИО. initialSolution должен быть валидной (и будет скопирован).
    unique_ptr<Solution> run(const Solution &initial) {
        // рабочие копии
        //unique_ptr<Solution> current = initial.clone();
        unique_ptr<Solution> best_solution = initial.clone();
        //double current_criteria = current->criteria();
        double best_solution_criteria = best_solution->criteria();

        double T = T0;
        int iter = 0;
        int noImprove = 0;

        while (iter < maxIterations && noImprove < noImproveLimit) {
            // создаём кандидата
            unique_ptr<Solution> new_solution = best_solution->clone();
            // применяем мутацию (in-place)
            mutation->apply(*new_solution, rng);

            double new_solution_criteria = new_solution->criteria();
            //double delta = new_solution_criteria - current_criteria;

            std::cout << "New ||| Old" << std::endl;
            std::cout << new_solution_criteria << " ||| " << best_solution_criteria << "\n" << std::endl;

            bool accept = false;
            if (new_solution_criteria < best_solution_criteria) {

                best_solution_criteria = new_solution_criteria;
                noImprove = 0;
                best_solution = move(new_solution);


            } else {
                // std::cout << "L1\n";
                double acceptanceProbability = std::exp(-(new_solution_criteria - best_solution_criteria) / T);
                
                // std::cout << acceptanceProbability << ' ' << prob << std::endl;
                if (acceptanceProbability >= static_cast<double>(rand()) / RAND_MAX)
                {
                    // Принимаем новое решение
                    
                    noImprove = 0;
                    best_solution = move(new_solution);

                }
                else
                {
                    // Не принимаем новое решение
                    noImprove++;
                }
            }

            // обновление температуры и увеличение счётчика
            ++iter;
            T = cooling->nextTemperature(T, iter);
        }

        return best_solution;
    }
};

// ------------------------------ Генератор входных данных ------------------------------
struct GeneratorConfig {
    int N;
    int M;
    int minW;
    int maxW;
};

vector<int> generateDurations(int N, int minW, int maxW, mt19937 &rng) {
    uniform_int_distribution<int> d(minW, maxW);
    vector<int> w(N);
    for (int i = 0; i < N; ++i) w[i] = d(rng);
    return w;
}

// Сохранить в CSV: первая строка N,M, вторая — длительности
void saveCSV(const string &filename, int N, int M, const vector<int> &w) {
    ofstream out(filename);
    out << "N,M\n" << N << "," << M << "\n";
    out << "durations\n";
    for (size_t i = 0; i < w.size(); ++i) {
        out << w[i];
        if (i + 1 < w.size()) out << ",";
    }
    out << "\n";
    out.close();
}

// ------------------------------ Пример использования в main ------------------------------
int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Параметры (можно менять):
    int N = 5;      // число работ
    int M = 2;       // число процессоров
    int minW = 1, maxW = 20;
    uint32_t seed = 2; // 0 -> случайно, иначе фиксированный seed

    // Если пользователь передал аргументы: N M seed
    if (argc >= 3) {
        N = stoi(argv[1]);
        M = stoi(argv[2]);
    }
    if (argc >= 4) seed = (uint32_t)stoul(argv[3]);

    random_device rd;
    if (seed == 0) seed = rd();
    mt19937 rng(seed);

    auto w = generateDurations(N, minW, maxW, rng);
    // Сгенерируем начальное случайное решение
    auto initial = ScheduleSolution::randomInit(N, M, w, rng);

    cout << "Initial solution:\n" << initial->toString() << "\n";

    // Выберем мутации: смесь swap и move
    vector<shared_ptr<Mutation>> muts;
    muts.push_back(make_shared<SwapTwoJobs>());
    muts.push_back(make_shared<MoveJob>());
    shared_ptr<Mutation> composite = make_shared<CompositeMutation>(muts);

    // Параметры ИО
    double T0 = 100.0;       // начальная температура (подбирать экспериментально)
    double Tmin = 1e-6;
    int maxIter = 1000000;   // верхний предел итераций (включает внутренние шаги)
    int NO_IMPROVE_LIMIT = 100; // K=100 по заданию

    // Три закона охлождения: выбери один (например exponential)
    unique_ptr<CoolingLaw> cooling = make_unique<CauchyCooling>(T0);

    // Альтернативы:
    // unique_ptr<CoolingLaw> cooling = make_unique<LinearCooling>(0.5);
    // unique_ptr<CoolingLaw> cooling = make_unique<LogarithmicCooling>(2.0);

    SimulatedAnnealing sa(T0, Tmin, maxIter, NO_IMPROVE_LIMIT, move(cooling), composite, seed);

    auto start = chrono::steady_clock::now();
    unique_ptr<Solution> best = sa.run(*initial);
    auto finish = chrono::steady_clock::now();
    chrono::duration<double> elapsed = finish - start;

    cout << "Best solution found (time " << elapsed.count() << " s):\n";
    cout << best->toString() << "\n";

    // Снять статистику и сохранить входные данные в CSV для отчёта
    saveCSV("input_gen.csv", N, M, w);
    cout << "Input saved to input_gen.csv\n";

    return 0;
}

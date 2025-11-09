/*
main2.cpp
Симуляция имитации отжига для задачи расписания N работ на M процессорах
Компиляция: g++ -std=c++17 main_parallel.cpp -O2 -o main_parallel
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

    ScheduleSolution() = default;

    ScheduleSolution(int N_, int M_, const vector<int> &w_) : N(N_), M(M_), w(w_) {
        jobLists.assign(M, {});
        // по умолчанию стартовая случайная раздача: round-robin
        for (int i = 0; i < N; ++i) {
            jobLists[i % M].push_back(i);
        }
    }
    
    // глубокая копия
    unique_ptr<Solution> clone() const override {
        auto s = make_unique<ScheduleSolution>();
        s->N = N; s->M = M; s->w = w;
        s->jobLists = jobLists;
        return s;
    }


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

    // текстовое представление решения
    string toString() const override {
        ostringstream oss;
        oss << "Schedule (M=" << M << ", N=" << N << "): (K1)=" << criteria() << "\n";

        // for (int j = 0; j < M; ++j) {
        //     int sum = 0;
        //     for (int id : jobLists[j])
        //         sum += w[id];

        //     oss << " CPU " << j << " (sum=" << sum << "): ";
        //     for (size_t k = 0; k < jobLists[j].size(); ++k) {
        //         int id = jobLists[j][k];
        //         oss << id << "(" << w[id] << ")";
        //         if (k + 1 < jobLists[j].size()) oss << ",";
        //     }
        //     oss << "\n";
        // }

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
    }

    // swap двух работ (p1,i1) и (p2,i2)
    void swapJobs(int p1, int i1, int p2, int i2) {
        if (p1 < 0 || p1 >= M || p2 < 0 || p2 >= M) return;
        if (i1 < 0 || i1 >= (int)jobLists[p1].size()) return;
        if (i2 < 0 || i2 >= (int)jobLists[p2].size()) return;
        std::swap(jobLists[p1][i1], jobLists[p2][i2]);
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
    int maxIterations; // макс. число итераций (внешних шагов)
    int noImproveLimit; // число итераций без улучшения для остановки (K = 100 по ТЗ)
    unique_ptr<CoolingLaw> cooling;
    shared_ptr<Mutation> mutation;
    mt19937 rng;

    SimulatedAnnealing(double T0_, int maxIter_, int noImproveLimit_,
                       unique_ptr<CoolingLaw> cooling_, shared_ptr<Mutation> mutation_, uint32_t seed = 0)
        : T0(T0_), maxIterations(maxIter_), noImproveLimit(noImproveLimit_),
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

        //std::cout << "best crit before loop = " << best_solution_criteria << std::endl;

        double T = T0;
        int iter = 0;
        int noImprove = 0;

        while (noImprove < noImproveLimit) {
            // создаём кандидата
            unique_ptr<Solution> new_solution = best_solution->clone();
            // применяем мутацию (in-place)
            mutation->apply(*new_solution, rng);

            double new_solution_criteria = new_solution->criteria();
            //double delta = new_solution_criteria - current_criteria;

            //std::cout << "New ||| Old" << std::endl;
            //std::cout << new_solution_criteria << " ||| " << best_solution_criteria << "\n" << std::endl;

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



// ------------------------------ Параллельная реализация ------------------------------
void parallelSimulatedAnnealing(
    const ScheduleSolution &initial,
    int Nproc,
    double T0,
    int maxIter,
    int noImproveLimit,
    shared_ptr<Mutation> mutation,
    const string &coolingType
) {
    mutex globalMutex;
    auto globalBest = initial.clone();
    double globalBestCriteria = globalBest->criteria();

    int globalNoImprove = 0;
    const int maxGlobalNoImprove = 10; // критерий останова по ТЗ

    while (globalNoImprove < maxGlobalNoImprove) {
        vector<thread> threads;
        vector<unique_ptr<Solution>> localBest(Nproc);

        for (int i = 0; i < Nproc; ++i) {
            threads.emplace_back([&, i]() {
                random_device rd;
                uint32_t seed = rd() + i * 100;

                // Так как каждый поток обязан иметь свои копии объектов, а не указатели на какие-то в памяти
                unique_ptr<CoolingLaw> cooling;
                if (coolingType == "Boltzmann")
                    cooling = make_unique<BoltzmannCooling>(T0);
                else if (coolingType == "Mixed")
                    cooling = make_unique<MixedCooling>(T0);
                else
                    cooling = make_unique<CauchyCooling>(T0);

                // каждый поток работает со своей копией текущего лучшего
                auto localInitial = dynamic_cast<ScheduleSolution*>(globalBest->clone().release());
                SimulatedAnnealing sa(T0, maxIter, noImproveLimit, move(cooling), mutation, seed);

                auto result = sa.run(*localInitial);
                localBest[i] = move(result);

                delete localInitial;
            });
        }

        for (auto &t : threads) t.join();

        bool improved = false;
        for (int i = 0; i < Nproc; ++i) {
            double crit = localBest[i]->criteria();
            lock_guard<mutex> lock(globalMutex);
            if (crit < globalBestCriteria) {
                globalBestCriteria = crit;
                globalBest = localBest[i]->clone();
                improved = true;
                cout << "[Iter] New global best = " << crit << "\n";
            }
        }

        if (improved) globalNoImprove = 0;
        else globalNoImprove++;
    }


    std::cout << globalBest->toString();
}



// ------------------------------ Генератор входных данных ------------------------------
vector<int> generateDurations(int N, int minW, int maxW, mt19937 &rng) {
    uniform_int_distribution<int> d(minW, maxW);
    vector<int> w(N);
    for (int i = 0; i < N; ++i) w[i] = d(rng);
    return w;
}


// ----------------------------- Парсинг данных из csv файла ---------------------------------
struct InputData {
    int N, M;
    string cooling;
    int minW, maxW;
    vector<int> w;
};

InputData readCSV(const string &filename) {
    InputData data;
    ifstream fin(filename);
    if (!fin.is_open())
        throw runtime_error("Не удалось открыть файл: " + filename);

    string line;

    // ---- первая строка ----
    if (!getline(fin, line))
        throw runtime_error("Ошибка: файл пустой");
    {
        stringstream ss(line);
        string field;
        vector<string> parts;
        while (getline(ss, field, ',')) parts.push_back(field);
        if (parts.size() < 5)
            throw runtime_error("Ошибка: первая строка должна содержать N,M,cooling,minW,maxW");
        data.N = stoi(parts[0]);
        data.M = stoi(parts[1]);
        data.cooling = parts[2];
        data.minW = stoi(parts[3]);
        data.maxW = stoi(parts[4]);
    }

    // ---- вторая строка ----
    if (!getline(fin, line))
        throw runtime_error("Ошибка: отсутствует строка с длительностями работ");
    {
        stringstream ss(line);
        string value;
        while (getline(ss, value, ',')) {
            if (!value.empty())
                data.w.push_back(stoi(value));
        }
        if ((int)data.w.size() != data.N)
            cerr << "⚠️ Предупреждение: количество длительностей (" << data.w.size()
                 << ") не совпадает с N=" << data.N << "\n";
    }

    return data;
}



// ------------------------------ Пример использования в main ------------------------------
int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N = 5, M = 2;
    int minW = 1, maxW = 20;
    uint32_t seed = 0;
    string mode = "auto";        // режим по умолчанию
    string coolingType = "Cauchy";
    vector<int> w;               // длительности работ


    random_device rd;
    if (seed == 0) seed = rd(); // Недетерминированность алгоритма
    mt19937 rng(rd());
    rng.seed(seed);



    /*
    Варианты запуска кода:
    1) Данные генерируются автоматически и/или указываются в коде
    2) Данные N, M, закон понижения температуры берутся из параметров и работы генерируются
    3) Данные N, M, закон понижения температуры и работы вводятся пользователем
    4) Данные N, M, закон понижения температуры и работы берутся из файла
    */
    // ------------------ Режимы ------------------
    if (argc == 1) {
        std::cout << "[Mode 1] Автоматическая генерация (по умолчанию)" << std::endl;
        N = 5; M = 2;
        w = generateDurations(N, minW, maxW, rng);
    }
    else if (argc == 5 && string(argv[1]) == "default") {
        std::cout << "[Mode 2] Аргументы командной строки" << std::endl;
        N = stoi(argv[2]);
        M = stoi(argv[3]);
        coolingType = argv[4];
        w = generateDurations(N, minW, maxW, rng);
    }
    else if (argc == 2 && string(argv[1]) == "manual") {
        std::cout << "[Mode 3] Ввод вручную" << std::endl;
        std::cout  << "Введите N (число работ) и M (число процессоров): " << std::endl;
        cin >> N >> M;
        std::cout  << "Введите закон охлаждения (Cauchy / Boltzmann / Mixed): " << std::endl;
        cin >> coolingType;
        std::cout  << "Введите длительности " << N << " работ: " << std::endl;
        w.resize(N);
        for (int i = 0; i < N; ++i) cin >> w[i];
    }
    else if (argc >= 3 && string(argv[1]) == "file") {
        std::cout  << "[Mode 4] Ввод из файла: " << argv[2] << std::endl;
        try {
            InputData data = readCSV(argv[2]);

            N = data.N;
            M = data.M;
            minW = data.minW;
            maxW = data.maxW;
            w = data.w;
            coolingType = data.cooling;

        }
        catch (const exception &e) {
            cerr << "Ошибка: " << e.what() << "\n";
            return 1;
        }
    } else {
        std::cerr << "Ошибка: неправильные аргументы.\n";
        std::cerr << "Использование:\n";
        std::cerr << "  ./main                     — авто режим\n";
        std::cerr << "  ./main default N M cooling — параметры из аргументов\n";
        std::cerr << "  ./main manual              — ввод вручную\n";
        std::cerr << "  ./main file input.txt      — ввод из файла\n";
        std::cout << std::endl;
        return 1;
    }



    // ------------------ Настройки -----------------

    std::cout << "\nПараметры:" << std::endl;
    std::cout << "  N = " << N << ", M = " << M << ", seed = " << seed << std::endl;
    std::cout << "  Закон охлаждения: " << coolingType << std::endl;
    std::cout << "  Времена работ: " << std::endl;
    //for (int t : w) cout << t << " ";
    std::cout << std::endl << std::endl;

    auto initial = ScheduleSolution(N, M, w);
    std::cout << "Initial solution:\n" << initial.toString() << std::endl;

    // Мутации
    vector<shared_ptr<Mutation>> muts = {
        make_shared<SwapTwoJobs>(),
        make_shared<MoveJob>()
    };
    shared_ptr<Mutation> composite = make_shared<CompositeMutation>(muts);

    // ------------------ Закон охлаждения ------------------
    double T0 = 100.0;
    int maxIter = 100000;
    int NO_IMPROVE_LIMIT = 100;
    unique_ptr<CoolingLaw> cooling;

    if (coolingType == "Boltzmann") {
        cooling = make_unique<BoltzmannCooling>(T0);
    } else if (coolingType == "Mixed") {
        cooling = make_unique<MixedCooling>(T0);
    } else if (coolingType == "Cauchy") {
        cooling = make_unique<CauchyCooling>(T0);
    } else {
        cerr << "Неизвестный тип охлаждения: " << coolingType << ". Используется Cauchy.\n\n";
        cooling = make_unique<CauchyCooling>(T0);
    }


    // --------------------------- Запуск параллельного ИО ----------------------------------
    int Nproc = 4; // например, 4 потока
    std::cout << "=== Параллельная версия (threads=" << Nproc << ") ===" << std::endl;
    auto start = chrono::steady_clock::now();
    
    parallelSimulatedAnnealing(initial, Nproc, T0, maxIter, NO_IMPROVE_LIMIT, composite, coolingType);
    
    auto finish = chrono::steady_clock::now();
    chrono::duration<double> elapsed = finish - start;
    std::cout << "Общее время работы: " << elapsed.count() << " секунд" << std::endl << std::endl;

    
    // // ------------------ Запуск последовательного ИО ------------------
    // std::cout << "=== Последовательная версия (threads=" << 1 << ") ===" << std::endl;
    // SimulatedAnnealing sa(T0, maxIter, NO_IMPROVE_LIMIT, move(cooling), composite, seed);
    // start = chrono::steady_clock::now();
    
    // unique_ptr<Solution> best = sa.run(initial);
    
    // finish = chrono::steady_clock::now();
    // elapsed = finish - start;

    // cout << best->toString();
    // std::cout << "Общее время работы: " << elapsed.count() << " секунд" << std::endl;
    return 0;
}

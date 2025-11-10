/*
main_parallel.cpp
Симуляция имитации отжига для задачи расписания N работ на M процессорах
Компиляция: g++ -std=c++17 main_parallel.cpp -O2 -o main_parallel
*/

#include <bits/stdc++.h>
#include "headers/abstruct.h"
#include "headers/solution.h"
#include "headers/cooling_laws.h"
#include "headers_parallel/head_class_parallel.h"
#include "headers_parallel/parallel_loop.h"
#include "headers/data_io.h"
#include "headers/mutations.h"

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

    int Nproc = 4; // например, 4 потока

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

        Nproc = stoi(argv[3]);

    } else {
        std::cerr << "Ошибка: неправильные аргументы.\n";
        std::cerr << "Использование:\n";
        std::cerr << "  ./main                     — авто режим\n";
        std::cerr << "  ./main default N M cooling — параметры из аргументов\n";
        std::cerr << "  ./main manual              — ввод вручную\n";
        std::cerr << "  ./main file input.txt Nproc      — ввод из файла\n";
        std::cout << std::endl;
        return 1;
    }



    // ------------------ Настройки -----------------
    std::cout << "\nПараметры:" << std::endl;
    std::cout << "=== Параллельная версия (threads=" << Nproc << ") ===" << std::endl;
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

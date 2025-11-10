using namespace std;

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
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
        // Надо ли вот это?
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

        while (iter < maxIterations && noImprove < noImproveLimit) {
            //if (iter % 1000 == 0) std::cout << iter << std::endl;
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
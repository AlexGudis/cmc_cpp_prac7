using namespace std;

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

using namespace std;

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
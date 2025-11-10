# pragma once

#include <bits/stdc++.h>
using namespace std;

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
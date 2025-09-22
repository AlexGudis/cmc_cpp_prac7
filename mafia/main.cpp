// main.cpp
// Требует C++20 (корутины). Компиляция:
// g++ -std=c++20 -O2 main.cpp -o mafia
// (если используешь clang, убедись, что корутины поддерживаются).
//
// Замена SharedPtr: в секции "SharedPtr alias" можно заменить std::shared_ptr на твою реализацию.

#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <optional>
#include <atomic>
#include <mutex>
#include <chrono>
#include <random>
#include <coroutine>

namespace fs = std::filesystem;
using namespace std::chrono_literals;
std::mt19937 rng((unsigned)std::chrono::system_clock::now().time_since_epoch().count());

// ============ SharedPtr alias ============
// Подставь сюда свою реализацию smart pointer, если хочешь.
// Например: template<typename T> using SharedPtr = MySharedPtr<T>;
template<typename T> using SharedPtr = std::shared_ptr<T>;
// ==========================================

std::mutex cout_mtx;
void cout_locked(const std::string &s){
    std::lock_guard lk(cout_mtx);
    std::cout << s << std::endl;
}

std::string now_ts(){
    auto t = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&tt));
    return std::string(buf);
}

// ================= Logger =================
struct Logger {
    fs::path dir;
    std::ofstream round_file;
    std::ofstream summary_file;
    std::mutex m;
    int round_idx = 0;

    Logger(const std::string &d="logs") : dir(d) {
        fs::create_directories(dir);
        summary_file.open(dir / "summary.log", std::ios::app);
        if (!summary_file) throw std::runtime_error("can't open summary.log");
    }

    void start_round() {
        std::lock_guard lk(m);
        ++round_idx;
        if (round_file.is_open()) round_file.close();
        auto p = dir / ("round_" + std::to_string(round_idx) + ".log");
        round_file.open(p);
    }

    void log_round(const std::string &s) {
        std::lock_guard lk(m);
        if (round_file) {
            round_file << now_ts() << " " << s << "\n";
            round_file.flush();
        }
    }

    void log_summary(const std::string &s) {
        std::lock_guard lk(m);
        summary_file << now_ts() << " " << s << "\n";
        summary_file.flush();
    }
};

// ================= Scheduler + Task =================
// Простейший кооперативный планировщик.
// Task для void и Task<T> для возвращаемого значения.

struct Scheduler {
    std::deque<std::coroutine_handle<>> q;
    std::mutex m;
    static Scheduler &instance(){
        static Scheduler s;
        return s;
    }
    void enqueue(std::coroutine_handle<> h){
        std::lock_guard lk(m);
        q.push_back(h);
    }
    void run(){
        while (true){
            std::coroutine_handle<> h = nullptr;
            {
                std::lock_guard lk(m);
                if (q.empty()) break;
                h = q.front(); q.pop_front();
            }
            if (h && !h.done()) h.resume();
        }
    }
    bool empty(){
        std::lock_guard lk(m);
        return q.empty();
    }
};

// Awaitable, который на старте регистрирует корутину в Scheduler
struct ScheduleOnStart {
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const noexcept {
        Scheduler::instance().enqueue(h);
    }
    void await_resume() const noexcept {}
};

// Yield — вернуть управление в планировщик и поставить корутину в конец очереди
struct Yield {
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const noexcept {
        Scheduler::instance().enqueue(h);
    }
    void await_resume() const noexcept {}
};

// InputAwaitable — при await чтение строки из stdin, затем возобновление корутины.
// Это синхронный ввод (блокирует stdin), но интегрирован с планировщиком.
struct InputAwaitable {
    std::string prompt;
    std::string result;
    InputAwaitable(std::string p=""): prompt(std::move(p)) {}
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        // Блокируем ввод, читаем строку, затем ставим coroutine в очередь
        {
            std::lock_guard lk(cout_mtx);
            if (!prompt.empty()) std::cout << prompt << std::flush;
        }
        std::string line;
        std::getline(std::cin, line);
        result = line;
        Scheduler::instance().enqueue(h);
    }
    std::string await_resume() noexcept { return result; }
};

// Task (void)
struct Task {
    struct promise_type {
        Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        ScheduleOnStart initial_suspend() noexcept { return {}; } // сразу регистрируемся в планировщике
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception(){ std::terminate(); }
    };
    std::coroutine_handle<promise_type> h;
    Task(std::coroutine_handle<promise_type> h_): h(h_) {}
    Task(Task&& o): h(o.h){ o.h = {}; }
    ~Task(){ if (h) h.destroy(); }
};

// Task<T>
template<typename T>
struct TaskT {
    struct promise_type {
        std::optional<T> value;
        TaskT get_return_object() { return TaskT{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        ScheduleOnStart initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_value(T v) noexcept { value = std::move(v); }
        void unhandled_exception(){ std::terminate(); }
    };
    std::coroutine_handle<promise_type> h;
    TaskT(std::coroutine_handle<promise_type> h_): h(h_) {}
    TaskT(TaskT&& o): h(o.h){ o.h = {}; }
    ~TaskT(){ if (h) h.destroy(); }
    // get result synchronously (after resuming)
    T get() {
        if (h && !h.done()) { Scheduler::instance().enqueue(h); Scheduler::instance().run(); }
        return std::move(*h.promise().value);
    }
};
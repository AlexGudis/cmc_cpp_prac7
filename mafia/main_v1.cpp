// main.cpp
// Требует C++20. Компиляция: g++ -std=c++20 -O2 main.cpp -pthread -o mafia

#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <optional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

namespace fs = std::filesystem;
using namespace std::chrono_literals;

// ===========================
// Подключи сюда свой shared_ptr:
// ===========================
template<typename T> using SharedPtr = std::shared_ptr<T>;
// Если у тебя класс называется, например, MySharedPtr, замени на:
// template<typename T> using SharedPtr = MySharedPtr<T>;

// ===========================
// Константы и утилиты
// ===========================
std::mt19937 rng((unsigned)std::chrono::system_clock::now().time_since_epoch().count());

std::mutex cout_mtx;
void log_console(const std::string &s){
    std::lock_guard<std::mutex> g(cout_mtx);
    std::cout << s << std::endl;
}

std::string now_ts() {
    auto t = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&tt));
    return std::string(buf);
}

// ===========================
// Концепты для ролей
// ===========================
template<typename T>
concept PlayerRole = requires(T a) {
    { a.name() } -> std::convertible_to<std::string>;
    { a.is_alive() } -> std::convertible_to<bool>;
    { a.act_night() } -> std::same_as<void>;
    { a.vote() } -> std::convertible_to<int>; // возвращает id выбранного игрока
};

// ===========================
// Логирование в файлы
// ===========================
struct Logger {
    fs::path dir;
    std::ofstream round_file;
    std::ofstream summary_file;
    std::mutex m;
    int round_idx = 0;

    Logger(const std::string &dirpath="logs") : dir(dirpath) {
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

// ===========================
// Игроки и роли
// ===========================
enum class RoleType { MAFIA, CITIZEN, COMMISSAR, DOCTOR, MANIAC, UNKNOWN };

struct GameState; // вперед

struct BasePlayer {
    int id;
    std::string nickname;
    RoleType role;
    std::atomic<bool> alive{true};
    std::mutex mtx;

    SharedPtr<GameState> gs;
    Logger *logger = nullptr;

    BasePlayer(int id_, std::string nick, RoleType r) : id(id_), nickname(std::move(nick)), role(r) {}
    virtual ~BasePlayer() = default;

    std::string name() const { return nickname + "(" + std::to_string(id) + ")"; }
    bool is_alive() const { return alive.load(); }

    virtual void act_night() = 0; // действия ночью (мафия выбирает жертву и т.д.)
    virtual int vote() = 0; // голосование днем: возвращает id выбранного
};

// forward: GameState
struct GameState {
    int N;
    std::vector<SharedPtr<BasePlayer>> players;
    std::mutex gs_mtx;

    int day = 0;
    Logger *logger = nullptr;

    // ночные цели
    std::optional<int> mafia_target;
    std::optional<int> doctor_target;
    std::optional<int> commissar_probe;
    std::optional<int> maniac_target;

    std::map<int, int> votes; // id -> count

    GameState(int N_): N(N_) {}
};

// === Роли ===
// Мафия: выбирает жертву ночью совместно (для простоты: каждая мафия голосует, голос большинства) 
struct Mafia : BasePlayer {
    Mafia(int id_, std::string nick): BasePlayer(id_, nick, RoleType::MAFIA) {}
    void act_night() override {
        if (!is_alive()) return;
        // выбирает случайную цель (не самого себя, не мертвого мафию)
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive() && p->id != id) candidates.push_back(p->id);
        }
        if (candidates.empty()) return;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int pick = candidates[d(rng)];
        // аккумулируем в mafia target (простейшая схема: последняя запись — итог)
        gs->mafia_target = pick;
        logger->log_round("Мафия " + name() + " выбрала в цель: " + std::to_string(pick));
    }
    int vote() override {
        // днём мафия может голосовать случайно
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive() && p->id != id) candidates.push_back(p->id);
        }
        if (candidates.empty()) return id;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int pick = candidates[d(rng)];
        logger->log_round("Мафия " + name() + " проголосовал за: " + std::to_string(pick));
        return pick;
    }
};

// Мирный житель
struct Citizen : BasePlayer {
    Citizen(int id_, std::string nick): BasePlayer(id_, nick, RoleType::CITIZEN) {}
    void act_night() override {
        // ничего не делает
        (void)0;
    }
    int vote() override {
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive() && p->id != id) candidates.push_back(p->id);
        }
        if (candidates.empty()) return id;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int pick = candidates[d(rng)];
        logger->log_round("Мирный " + name() + " проголосовал за: " + std::to_string(pick));
        return pick;
    }
};

// Комиссар — проверяет роль цели ночью
struct Commissar : BasePlayer {
    Commissar(int id_, std::string nick): BasePlayer(id_, nick, RoleType::COMMISSAR) {}
    void act_night() override {
        if (!is_alive()) return;
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive() && p->id != id) candidates.push_back(p->id);
        }
        if (candidates.empty()) return;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int probe = candidates[d(rng)];
        gs->commissar_probe = probe;
        logger->log_round("Комиссар " + name() + " проверил: " + std::to_string(probe));
    }
    int vote() override {
        // простая логика
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive() && p->id != id) candidates.push_back(p->id);
        }
        if (candidates.empty()) return id;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int pick = candidates[d(rng)];
        logger->log_round("Комиссар " + name() + " проголосовал за: " + std::to_string(pick));
        return pick;
    }
};

// Доктор — лечит ночью (предотвращает убийство)
struct Doctor : BasePlayer {
    Doctor(int id_, std::string nick): BasePlayer(id_, nick, RoleType::DOCTOR) {}
    void act_night() override {
        if (!is_alive()) return;
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive()) candidates.push_back(p->id);
        }
        if (candidates.empty()) return;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int heal = candidates[d(rng)];
        gs->doctor_target = heal;
        logger->log_round("Доктор " + name() + " лечит: " + std::to_string(heal));
    }
    int vote() override {
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive() && p->id != id) candidates.push_back(p->id);
        }
        if (candidates.empty()) return id;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int pick = candidates[d(rng)];
        logger->log_round("Доктор " + name() + " проголосовал за: " + std::to_string(pick));
        return pick;
    }
};

// Маньяк — ночью может убивать самостоятельно
struct Maniac : BasePlayer {
    Maniac(int id_, std::string nick): BasePlayer(id_, nick, RoleType::MANIAC) {}
    void act_night() override {
        if (!is_alive()) return;
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive() && p->id != id) candidates.push_back(p->id);
        }
        if (candidates.empty()) return;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int t = candidates[d(rng)];
        gs->maniac_target = t;
        logger->log_round("Маньяк " + name() + " выбрал цель: " + std::to_string(t));
    }
    int vote() override {
        std::vector<int> candidates;
        {
            std::lock_guard lk(gs->gs_mtx);
            for (auto &p: gs->players) if (p->is_alive() && p->id != id) candidates.push_back(p->id);
        }
        if (candidates.empty()) return id;
        std::uniform_int_distribution<int> d(0, (int)candidates.size()-1);
        int pick = candidates[d(rng)];
        logger->log_round("Маньяк " + name() + " проголосовал за: " + std::to_string(pick));
        return pick;
    }
};


struct HumanPlayer : BasePlayer {
    SharedPtr<BasePlayer> role_impl; // делегат на реальную роль (Mafia/Commissar/..)

    HumanPlayer(int id_, std::string nick, SharedPtr<BasePlayer> impl)
        : BasePlayer(id_, nick, impl->role), role_impl(std::move(impl)) {}

    Task act_night() override {
        co_await Yield{};
        if (!is_alive()) co_return;
        // Предлагаем опции в зависимости от роли
        std::string prompt;
        if (role == RoleType::MAFIA) {
            prompt = "Твоя ночь (мафия). Введи id цели (или empty -> рандом): ";
            std::string line = co_await InputAwaitable(prompt);
            if (line.empty()) {
                // делегируем AI выбор (вызов оригинальной реализации)
                co_await role_impl->act_night();
            } else {
                int t = std::stoi(line);
                role_impl->gs->mafia_target = t;
                logger->log_round("Человек (мафия) выбрал цель: " + std::to_string(t));
            }
        } else if (role == RoleType::COMMISSAR) {
            // выбор: probe или kill
            std::string line = co_await InputAwaitable("Ты комиссар. Введи 'probe id' или 'kill id' (например: probe 3): ");
            if (line.empty()) {
                co_await role_impl->act_night();
            } else {
                std::istringstream iss(line);
                std::string cmd; int tid;
                iss >> cmd >> tid;
                if (cmd=="kill") {
                    role_impl->gs->commissar_target = tid;
                    role_impl->gs->commissar_kill = true;
                    logger->log_round("Человек (комиссар) ночью убил: " + std::to_string(tid));
                } else {
                    role_impl->gs->commissar_target = tid;
                    role_impl->gs->commissar_kill = false;
                    logger->log_round("Человек (комиссар) проверил: " + std::to_string(tid));
                }
            }
        } else if (role == RoleType::DOCTOR) {
            std::string line = co_await InputAwaitable("Ты доктор. Введи id для лечения (или empty -> рандом): ");
            if (line.empty()) co_await role_impl->act_night();
            else { int t = std::stoi(line); role_impl->gs->doctor_target = t; logger->log_round("Человек (доктор) лечит: " + std::to_string(t)); }
        } else if (role == RoleType::MANIAC) {
            std::string line = co_await InputAwaitable("Ты маньяк. Введи id цели (или empty -> рандом): ");
            if (line.empty()) co_await role_impl->act_night();
            else { int t = std::stoi(line); role_impl->gs->maniac_target = t; logger->log_round("Человек (маньяк) цель: " + std::to_string(t)); }
        } else if (role == RoleType::CITIZEN) {
            co_await Yield{};
        }
        co_return;
    }

    TaskT<int> vote() override {
        co_await Yield{};
        if (!is_alive()) co_return id;
        std::string line = co_await InputAwaitable("Днём — введи id за кого голосуешь: ");
        if (line.empty()) {
            // случайный выбор
            int pick = id;
            std::vector<int> cand;
            for (auto &p: gs->players) if (p->is_alive() && p->id!=id) cand.push_back(p->id);
            if (!cand.empty()) { std::uniform_int_distribution<int> d(0,(int)cand.size()-1); pick = cand[d(rng)]; }
            logger->log_round("Человек проголосовал случайно за " + std::to_string(pick));
            co_return pick;
        } else {
            int pick = std::stoi(line);
            logger->log_round("Человек проголосовал за " + std::to_string(pick));
            co_return pick;
        }
    }
};


// ===========================
// Вспомогательные функции
// ===========================
std::string role_to_str(RoleType r) {
    switch(r){
        case RoleType::MAFIA: return "Mafia";
        case RoleType::CITIZEN: return "Citizen";
        case RoleType::COMMISSAR: return "Commissar";
        case RoleType::DOCTOR: return "Doctor";
        case RoleType::MANIAC: return "Maniac";
        default: return "Unknown";
    }
}

int count_alive(const GameState &gs) {
    int c = 0;
    for (auto &p: gs.players) if (p->is_alive()) ++c;
    return c;
}

int count_role_alive(const GameState &gs, RoleType rt) {
    int c = 0;
    for (auto &p: gs.players) if (p->is_alive() && p->role==rt) ++c;
    return c;
}

// ===========================
// Модератор / Ведущий
// ===========================
struct Moderator {
    SharedPtr<GameState> gs;
    Logger *logger;
    bool human_player = false;
    int human_id = -1;

    Moderator(SharedPtr<GameState> gs_, Logger *log): gs(gs_), logger(log) {}

    void run_game(bool full_log=false) {
        logger->start_round();
        while (true) {
            gs->day++;
            logger->start_round();
            logger->log_round("=== Ночь " + std::to_string(gs->day) + " начинается ===");
            // ночь: все живые выполняют act_night (в отдельных нитях)
            std::vector<std::thread> threads;
            {
                std::lock_guard lk(gs->gs_mtx);
                for (auto &p : gs->players) {
                    if (!p->is_alive()) continue;
                    threads.emplace_back([p](){
                        p->act_night();
                    });
                }
            }
            for (auto &t: threads) if (t.joinable()) t.join();
            // ночные результаты
            resolve_night();

            if (check_end()) break;

            // день: голосование
            logger->log_round("=== День " + std::to_string(gs->day) + " — голосование ===");
            gs->votes.clear();
            {
                std::lock_guard lk(gs->gs_mtx);
                for (auto &p: gs->players) if (p->is_alive()) {
                    int picked = p->vote();
                    gs->votes[picked]++;
                }
            }
            // подсчёт голосов
            int maxvotes = 0;
            int kicked = -1;
            for (auto &kv: gs->votes) {
                if (kv.second > maxvotes) { maxvotes = kv.second; kicked = kv.first; }
            }
            if (kicked != -1) {
                // убиваем игрока с id == kicked (если он жив)
                std::lock_guard lk(gs->gs_mtx);
                for (auto &p: gs->players) {
                    if (p->id == kicked && p->is_alive()) {
                        p->alive = false;
                        logger->log_round("Игрок " + p->name() + " был изгнан голосованием. Роль: " + role_to_str(p->role));
                    }
                }
            } else {
                logger->log_round("Никого не изгнали (ничья).");
            }

            if (check_end()) break;
        }
        // итог
        logger->log_summary("Игра закончена. День: " + std::to_string(gs->day));
        // summary roles
        for (auto &p: gs->players) {
            logger->log_summary(p->name() + " role=" + role_to_str(p->role) + " alive=" + (p->is_alive() ? "1" : "0"));
        }
    }

    void resolve_night() {
        std::lock_guard lk(gs->gs_mtx);
        logger->log_round("Ночные цели: mafia->" + (gs->mafia_target ? std::to_string(*gs->mafia_target) : std::string("none"))
                                         + " doctor->" + (gs->doctor_target ? std::to_string(*gs->doctor_target) : std::string("none"))
                                         + " maniac->" + (gs->maniac_target ? std::to_string(*gs->maniac_target) : std::string("none"))
                                         + " commissar->" + (gs->commissar_probe ? std::to_string(*gs->commissar_probe) : std::string("none")));
        // приоритет/логика:
        // 1) если доктор лечит выбранную мафией цель, убийство предотвращается
        // 2) маньяк действует независимо (может убить того, кого доктор лечит)
        std::optional<int> night_victim;
        if (gs->mafia_target) night_victim = *gs->mafia_target;
        // маньяк
        if (gs->maniac_target) {
            // если маньяк выбрал цель, она также умирает (если не лечат)
            if (!night_victim) night_victim = *gs->maniac_target;
            else {
                // оба выбрали — считаем, что обе попытки приводят к смерти цели (при наличии лечения)
                // для простоты: мафия цель и маньяк цель обрабатываются отдельно ниже
            }
        }

        // убиваем мафией цель, если доктор не лечил её
        if (gs->mafia_target) {
            int t = *gs->mafia_target;
            if (!(gs->doctor_target && *gs->doctor_target == t)) {
                for (auto &p: gs->players) {
                    if (p->id == t && p->is_alive()) {
                        p->alive = false;
                        logger->log_round("Игрок " + p->name() + " был убит ночью (мафия). Роль: " + role_to_str(p->role));
                    }
                }
            } else {
                logger->log_round("Доктор спас игрока " + std::to_string(t) + " от мафии.");
            }
        }

        // маньяк
        if (gs->maniac_target) {
            int t = *gs->maniac_target;
            if (!(gs->doctor_target && *gs->doctor_target == t)) {
                for (auto &p: gs->players) {
                    if (p->id == t && p->is_alive()) {
                        p->alive = false;
                        logger->log_round("Игрок " + p->name() + " был убит ночью (маньяк). Роль: " + role_to_str(p->role));
                    }
                }
            } else {
                logger->log_round("Доктор спас игрока " + std::to_string(t) + " от маньяка.");
            }
        }

        // комиссар: раскрываем роль проверенной цели
        if (gs->commissar_probe) {
            int t = *gs->commissar_probe;
            for (auto &p: gs->players) if (p->id == t) {
                logger->log_round("Комиссар узнал роль игрока " + p->name() + " : " + role_to_str(p->role));
            }
        }

        // обнуляем ночные цели
        gs->mafia_target.reset(); gs->doctor_target.reset(); gs->commissar_probe.reset(); gs->maniac_target.reset();
    }

    bool check_end() {
        // Победа мафии — когда они равны по численности мирным (или больше)
        int mafia_alive = count_role_alive(*gs, RoleType::MAFIA);
        int others_alive = count_alive(*gs) - mafia_alive;
        if (mafia_alive == 0) {
            logger->log_summary("Победа мирных. Мафия уничтожена.");
            return true;
        }
        if (mafia_alive >= others_alive) {
            logger->log_summary("Победа мафии.");
            return true;
        }
        // маньяк может выиграть сам — если остался один живой и он маньяк
        int alive_total = count_alive(*gs);
        if (alive_total == 1) {
            for (auto &p: gs->players) if (p->is_alive() && p->role == RoleType::MANIAC) {
                logger->log_summary("Победа маньяка.");
                return true;
            }
        }
        return false;
    }
};

// ===========================
// Конфигурация YAML (очень простой парсер)
// ===========================
struct Config {
    // map role_name -> count (if count==0 -> auto)
    std::map<std::string,int> roles;
    bool human = false;
    bool full_log = false;
};

Config load_config(const fs::path &p) {
    Config c;
    if (!fs::exists(p)) return c;
    std::ifstream in(p);
    std::string line;
    while (std::getline(in, line)) {
        std::string s = line;
        // trim
        while(!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
        while(!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
        if (s.empty() || s[0]=='#' || s[0]=='-') continue;
        // простой парсинг вида: key: value
        auto pos = s.find(':');
        if (pos==std::string::npos) continue;
        std::string key = s.substr(0,pos);
        std::string val = s.substr(pos+1);
        // trim
        while(!key.empty() && isspace((unsigned char)key.back())) key.pop_back();
        while(!key.empty() && isspace((unsigned char)key.front())) key.erase(key.begin());
        while(!val.empty() && isspace((unsigned char)val.back())) val.pop_back();
        while(!val.empty() && isspace((unsigned char)val.front())) val.erase(val.begin());
        if (key == "human") c.human = (val == "true" || val == "1" || val=="yes");
        else if (key == "full_log") c.full_log = (val == "true" || val == "1" || val=="yes");
        else {
            // role: N
            try {
                int cnt = std::stoi(val);
                c.roles[key] = cnt;
            } catch(...) {
                c.roles[key] = 0;
            }
        }
    }
    return c;
}

// ===========================
// Фабрика ролей по имени
// ===========================
SharedPtr<BasePlayer> make_role(int id, const std::string &name, const std::string &role_name) {
    if (role_name == "Mafia") return std::make_shared<Mafia>(id, name);
    if (role_name == "Citizen") return std::make_shared<Citizen>(id, name);
    if (role_name == "Commissar") return std::make_shared<Commissar>(id, name);
    if (role_name == "Doctor") return std::make_shared<Doctor>(id, name);
    if (role_name == "Maniac") return std::make_shared<Maniac>(id, name);
    // default
    return std::make_shared<Citizen>(id, name);
}

// ===========================
// Main: сборка игроков, запуск
// ===========================
int main(int argc, char** argv) {
    int N = 10;
    int K = 3; // N/k -> mafia count
    bool human = false;
    bool full_log = false;
    fs::path cfgpath = "config.yaml";

    // простой парсинг аргументов
    for (int i=1;i<argc;i++){
        std::string a = argv[i];
        if (a=="-n" && i+1<argc) N = std::stoi(argv[++i]);
        else if (a=="-k" && i+1<argc) K = std::stoi(argv[++i]);
        else if (a=="--human") human = true;
        else if (a=="--logfull") full_log = true;
        else if (a=="--config" && i+1<argc) cfgpath = argv[++i];
    }

    Config cfg = load_config(cfgpath);
    if (cfg.human) human = true;
    if (cfg.full_log) full_log = true;

    Logger logger("logs");
    auto gs = std::make_shared<GameState>(N);
    gs->logger = &logger;

    // Формируем роли: базовый набор
    std::vector<std::string> role_pool;
    // если cfg.roles заполнен — используем его, иначе дефолт
    if (!cfg.roles.empty()) {
        // cfg.roles may specify counts; if count==0 — auto-fill as citizens
        for (auto &kv: cfg.roles) {
            const std::string &rname = kv.first;
            int cnt = kv.second;
            if (cnt > 0) {
                for (int i=0;i<cnt;i++) role_pool.push_back(rname);
            }
        }
    } else {
        // default minimal composition: will set precise counts below
    }

    // compute mafia count
    int mafia_count = std::max(1, N / K);

    // ensure required roles are present: Commissar, Doctor, Maniac (one each)
    // We'll create the list of role names and then shuffle/assign.
    std::vector<std::string> roles;
    roles.reserve(N);
    // put mafia_count mafias
    for (int i=0;i<mafia_count;i++) roles.push_back("Mafia");
    // mandatory roles
    roles.push_back("Commissar");
    roles.push_back("Doctor");
    roles.push_back("Maniac");
    // rest citizens
    while ((int)roles.size() < N) roles.push_back("Citizen");

    // If YAML provided explicit roles counts (some may be extra roles), fill them too:
    if (!role_pool.empty()) {
        // replace first role_pool.size() entries by role_pool (or append)
        roles.clear();
        for (auto &r: role_pool) roles.push_back(r);
        while ((int)roles.size() < N) roles.push_back("Citizen");
    }

    std::shuffle(roles.begin(), roles.end(), rng);

    // create players
    for (int i=0;i<N;i++) {
        std::string nick = "P" + std::to_string(i+1);
        auto p = make_role(i+1, nick, roles[i]);
        p->gs = gs;
        p->logger = &logger;
        gs->players.push_back(p);
    }

    // print composition
    {
        std::string s = "Состав игроков:";
        for (auto &p: gs->players) s += " " + p->name() + "[" + role_to_str(p->role) + "]";
        log_console(s);
        logger.log_summary(s);
    }

    Moderator mod(gs, &logger);
    mod.human_player = human;

    // Привяжем логгер к каждому игроку
    for (auto &p: gs->players) p->logger = &logger;

    // Запуск игры
    mod.run_game(full_log);

    log_console("Игра завершена. Логи в папке logs/");
    return 0;
}

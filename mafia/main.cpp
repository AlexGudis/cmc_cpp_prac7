#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <coroutine>
#include <future>
#include <string>
#include <ranges>

#include "formatter.cpp"
#include "shr_ptr.cpp"
#include "logger.cpp"

// Используем псевдоним для удобства работы с ranges
namespace view = std::ranges::views;

// ============================================================================
// КОРУТИНЫ: Реализация асинхронных задач для игровых действий
// ============================================================================

// Based on https://habr.com/ru/articles/798935/
struct promise;

// Корутина для представления игровых действий (голосование, ночные действия)
struct Task : std::coroutine_handle<promise> {
    using promise_type = ::promise;
    std::coroutine_handle<promise_type> handle;
};

// Promise-объект для управления корутиной
struct promise {
    // Создает объект корутины
    Task get_return_object() { return Task{}; }
    // Не приостанавливаем при старте - начинаем выполнение сразу
    std::suspend_never initial_suspend() noexcept { return {}; }
    // Не приостанавливаем при завершении - уничтожаем сразу
    std::suspend_never final_suspend() noexcept { return {}; }
    // Корутина не возвращает значения
    void return_void() {}
    // Обработка исключений (не реализована)
    void unhandled_exception() {}
};

// Вспомогательная функция для вывода с форматированием
template<typename T>
void print(T value) {
    std::cout << Format(value) << std::endl;
}

// Функция для случайного перемешивания элементов контейнера
template<typename T>
void simple_shuffle(T& container) {
    std::mt19937 g(rand());  // Генератор случайных чисел
    std::shuffle(container.begin(), container.end(), g);  // Перемешивание
}

// ============================================================================
// СТРУКТУРА ДЛЯ ХРАНЕНИЯ НОЧНЫХ ДЕЙСТВИЙ ИГРОКОВ
// ============================================================================

struct NightActions {
    unsigned int players_num;  // Общее количество игроков
    bool doctors_action;       // Флаг действия доктора
    unsigned int doctors_choice;  // Выбор доктора (кого лечить)
    bool commissar_action;     // Флаг действия комиссара
    unsigned int commissar_choice;  // Выбор комиссара (кого проверить)
    bool journalist_action;    // Флаг действия журналиста
    std::pair<unsigned int, unsigned int> journalist_choice;  // Пара игроков для проверки
    bool samurai_action;       // Флаг действия самурая
    unsigned int samurai_choice;  // Выбор самурая (кого защитить)
    
    // Вектор векторов: для каждого игрока храним список тех, кто на него напал
    std::vector<std::vector<unsigned int>> killers;
    
    // Конструктор инициализирует структуру для заданного количества игроков
    explicit NightActions(unsigned int players_num_) : players_num(players_num_) {
        // Создаем пустые списки атакующих для каждого игрока
        for (size_t i = 0; i < players_num; i++) {
            killers.push_back({});
        }
        // Инициализируем флаги действий как false
        commissar_action = false;
        doctors_action = false;
        journalist_action = false;
        samurai_action = false;
    }
    
    // Сброс всех действий для новой ночи
    void reset() {
        // Очищаем списки атакующих для каждого игрока
        for (size_t i = 0; i < killers.size(); i++) {
            killers[i].clear();
        }
        // Сбрасываем все флаги действий
        commissar_action = false;
        doctors_action = false;
        journalist_action = false;
        samurai_action = false;
    }
};

// ============================================================================
// БАЗОВЫЙ КЛАСС ИГРОКА И ЕГО СПЕЦИАЛИЗАЦИИ
// ============================================================================

// Абстрактный базовый класс для всех типов игроков
class Player {
public:
    // Конструктор инициализирует базовые параметры игрока
    Player(size_t id_p) : id(id_p) {
        alive = true;          // Игрок жив при создании
        is_real_player = false; // По умолчанию - компьютерный игрок
        is_boss = false;       // По умолчанию - не босс мафии
    };
    
    virtual ~Player() {};
    
    // Виртуальная функция голосования (возвращает корутину)
    virtual Task vote(std::vector<size_t> alive_ids, size_t& value) {
        if (is_real_player) {
            // Для реального игрока - вызываем интерактивную версию
            vote_player(alive_ids, value);
            co_return;  // Завершаем корутину
        } else {
            // Для AI - вызываем AI-версию
            vote_ai(alive_ids, value);
            co_return;
        }
    }
    
    // Виртуальная функция ночного действия (возвращает корутину)
    virtual Task act(std::vector<size_t> alive_ids,
                     NightActions& night_actions,
                     std::vector<Shared_pointer<Player>> players) {
        if (is_real_player) {
            act_player(alive_ids, night_actions, players);
            co_return;
        } else {
            act_ai(alive_ids, night_actions, players);
            co_return;
        }
    }
    
    // Чисто виртуальные функции, которые должны быть реализованы в потомках
    virtual void vote_ai(std::vector<size_t>& alive_ids, size_t& value) = 0;
    
    // Реализация голосования для реального игрока
    virtual void vote_player(std::vector<size_t>& alive_ids, size_t& value) {
        std::cout << "Voting! Choose which candidate to vote for from the following:" << std::endl;
        // Показываем доступных для голосования игроков
        for (auto i : alive_ids) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        size_t res;
        std::cin >> res;  // Получаем выбор игрока
        value = res;      // Сохраняем результат
        return;
    }
    
    virtual void act_ai(std::vector<size_t>& alive_ids,
                        NightActions& night_actions,
                        std::vector<Shared_pointer<Player>> players) = 0;
                        
    virtual void act_player(std::vector<size_t>& alive_ids,
                            NightActions& night_actions,
                            std::vector<Shared_pointer<Player>> players) = 0;

    // Параметры игрока
    bool alive;           // Жив ли игрок
    bool is_real_player;  // Является ли реальным игроком (человеком)
    bool is_boss;         // Является ли боссом мафии
    size_t id;            // Уникальный идентификатор игрока
    std::vector<size_t> known_mafia{};  // Список известных мафиози (для мафии и комиссара)
    std::string team;     // Команда ("civilian", "mafia", "maniac")
    std::string role;     // Роль ("civilian", "commissar", "doctor", etc.)
};

// ============================================================================
// КЛАСС МИРНОГО ЖИТЕЛЯ (БАЗОВЫЙ ДЛЯ ГОРОЖАН)
// ============================================================================

class Civilian : public Player {
public:
    Civilian(size_t id_p) : Player(id_p) {
        team = "civilian";   // Команда мирных жителей
        role = "civilian";   // Роль обычного жителя
    }
    virtual ~Civilian() {};

    // AI-логика голосования для мирного жителя
    virtual void vote_ai(std::vector<size_t>& alive_ids, size_t& value) override {
        simple_shuffle(alive_ids);  // Случайно перемешиваем список живых
        size_t i = 0;
        // Ищем первого живого игрока, который не является собой
        while (i < alive_ids.size()) {
            if (alive_ids[i] != id) {
                value = alive_ids[i];  // Голосуем против него
                return;
            }
            i++;
        }
        value = 0;  // Запасной вариант (не должен происходить)
        return;
    }
    
    // Мирные жители не совершают действий ночью (базовая реализация)
    virtual void act_ai(std::vector<size_t>&, NightActions&, std::vector<Shared_pointer<Player>>) override {
        return;
    }
    
    virtual void act_player(std::vector<size_t>&, NightActions&, std::vector<Shared_pointer<Player>>) override {
        return;
    }
};

// ============================================================================
// КЛАСС КОМИССАРА (ДЕТЕКТИВА)
// ============================================================================

class Commissar : public Civilian {
    std::vector<size_t> already_checked;  // Уже проверенные игроки
    std::vector<size_t> known_civilian;   // Известные мирные жители
public:
    Commissar(size_t id_p) : Civilian(id_p) {
        already_checked = {id};  // Начинаем с проверки себя
        known_civilian = {id};   // Себя знаем как мирного
        role = "commissar";      // Устанавливаем роль
    }
    virtual ~Commissar() {};

    // AI-логика голосования комиссара
    virtual void vote_ai(std::vector<size_t>& alive_ids, size_t& value) override {
        simple_shuffle(alive_ids);
        // Сначала голосуем против известных мафиози, если они живы
        for (size_t i = 0; i < known_mafia.size(); i++) {
            if (std::find(alive_ids.begin(), alive_ids.end(), known_mafia[i]) != alive_ids.end()) {
                value = known_mafia[i];  // Голосуем против мафиози
                return;
            }
        }
        // Если известных мафиози нет, голосуем как обычный житель
        Civilian::vote_ai(alive_ids, value);
        return;
    }
    
    // AI-логика ночных действий комиссара
    virtual void act_ai(std::vector<size_t>& alive_ids,
                        NightActions& night_actions,
                        std::vector<Shared_pointer<Player>> players) override {
        // Если есть известные мафиози среди живых - стреляем в них
        for (size_t i = 0; i < known_mafia.size(); i++) {
            if (std::find(alive_ids.begin(), alive_ids.end(), known_mafia[i]) != alive_ids.end()) {
                night_actions.killers[known_mafia[i]].push_back(id);
                return;
            }
        }
        
        // Иначе проверяем нового игрока
        auto max_id = *std::max_element(alive_ids.begin(), alive_ids.end());
        for (size_t i = 0; i <= max_id; i++) {
            // Ищем еще не проверенного игрока
            if (std::find(already_checked.begin(), already_checked.end(), i) == already_checked.end()) {
                already_checked.push_back(i);  // Добавляем в проверенные
                
                // Запоминаем результат проверки
                if (players[i]->team == "mafia") {
                    known_mafia.push_back(i);  // Запоминаем мафиози
                } else {
                    known_civilian.push_back(i);  // Запоминаем мирного
                }
                
                // Устанавливаем флаг действия комиссара
                night_actions.commissar_action = true;
                night_actions.commissar_choice = i;
                return;
            }
        }
        return;
    }
    
    // Интерактивная версия ночных действий для реального игрока
    virtual void act_player(std::vector<size_t>& alive_ids,
                            NightActions& night_actions,
                            std::vector<Shared_pointer<Player>> players) override {
        while (true) {
            std::cout << "Choose an action: shoot (s) or check (c)." << std::endl;
            std::string choice;
            std::cin >> choice;
            std::cout << "Choose one of:" << std::endl;
            for (auto i : alive_ids) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
            size_t shoot_check;
            std::cin >> shoot_check;
            
            if (choice == "shoot" || choice == "s") {
                // Выстрел в выбранного игрока
                night_actions.killers[shoot_check].push_back(id);
                return;
            } else if (choice == "check" || choice == "c") {
                // Проверка игрока
                std::cout << "Player " << shoot_check << " is "
                          << ((players[shoot_check]->team == "mafia") ? "mafia" : "not mafia") << std::endl;
                night_actions.commissar_action = true;
                night_actions.commissar_choice = shoot_check;
                return;
            } else {
                std::cout << "Incorrect action!" << std::endl;
            }
        }
    }
};

// ============================================================================
// КЛАСС ДОКТОРА
// ============================================================================

class Doctor : public Civilian {
    size_t last_heal;  // ID последнего вылеченного игрока
public:
    Doctor(size_t id_p) : Civilian(id_p) {
        role = "doctor";
        last_heal = std::numeric_limits<size_t>::max();  // Инициализируем максимальным значением
    }
    virtual ~Doctor() {}

    // AI-логика лечения доктора
    virtual void act_ai(std::vector<size_t>& alive_ids,
                        NightActions& night_actions,
                        std::vector<Shared_pointer<Player>>) override {
        simple_shuffle(alive_ids);
        // Ищем игрока, которого не лечили в прошлую ночь
        for (size_t i = 0; i < alive_ids.size(); i++) {
            if (alive_ids[i] != last_heal) {
                night_actions.doctors_action = true;
                night_actions.doctors_choice = alive_ids[i];
                last_heal = alive_ids[i];  // Запоминаем кого лечили
                return;
            }
        }
    }
    
    // Интерактивная версия лечения
    virtual void act_player(std::vector<size_t>& alive_ids,
                            NightActions& night_actions,
                            std::vector<Shared_pointer<Player>>) override {
        std::cout << "Who do you want to heal?" << std::endl
                  << "Choose one of:" << std::endl;
        for (auto i : alive_ids) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        std::cout << "But it shouldn't be the same player you healed last time." << std::endl;
        
        while (true) {
            size_t choice;
            std::cin >> choice;
            if (last_heal == choice) {
                std::cout << "You already healed this player last time." << std::endl;
                continue;
            } else {
                night_actions.doctors_action = true;
                night_actions.doctors_choice = choice;
                last_heal = choice;
                return;
            }
        }
    }
};

// ============================================================================
// КЛАСС ЖУРНАЛИСТА
// ============================================================================

class Journalist : public Civilian {
public:
    Journalist(size_t id_p) : Civilian(id_p) {
        role = "journalist";
    }
    virtual ~Journalist() {}

    // AI-логика журналиста: проверяет двух случайных игроков
    virtual void act_ai(std::vector<size_t>& alive_ids,
                        NightActions& night_actions,
                        std::vector<Shared_pointer<Player>>) override {
        simple_shuffle(alive_ids);
        // Перебираем все пары игроков (кроме себя)
        for (size_t i = 0; i < alive_ids.size() - 1; i++) {
            for (size_t j = i + 1; j < alive_ids.size(); j++) {
                if (i != id && j != id) {
                    night_actions.journalist_action = true;
                    night_actions.journalist_choice = {i, j};
                }
            }
        }
    }
    
    // Интерактивная версия проверки
    virtual void act_player(std::vector<size_t>& alive_ids,
                            NightEvents& night_actions,
                            std::vector<Shared_pointer<Player>> players) override {
        std::cout << "Choose two players to compare:" << std::endl;
        for (auto i : alive_ids) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        std::cout << "Cannot select yourself." << std::endl;
        
        while (true) {
            size_t first, second;
            std::cin >> first >> second;
            if (first != id && second != id) {
                // Проверяем, одной ли команды игроки
                if (players[first]->team == players[second]->team) {
                    std::cout << "Same faction" << std::endl;
                } else {
                    std::cout << "Different factions" << std::endl;
                }
                night_actions.journalist_action = true;
                night_actions.journalist_choice = {first, second};
                return;
            } else {
                std::cout << "Cannot select yourself!" << std::endl;
                continue;
            }
        }
    }
};

// ============================================================================
// КЛАСС САМУРАЯ (ОХРАННИКА)
// ============================================================================

class Samurai : public Civilian {
public:
    Samurai(size_t id_p) : Civilian(id_p) {
        role = "samurai";
    }
    virtual ~Samurai() {}

    // AI-логика самурая: защищает случайного игрока
    virtual void act_ai(std::vector<size_t>& alive_ids,
                        NightActions& night_actions,
                        std::vector<Shared_pointer<Player>>) override {
        simple_shuffle(alive_ids);
        for (size_t i = 0; i < alive_ids.size(); i++) {
            night_actions.samurai_action = true;
            night_actions.samurai_choice = i;
        }
    }
    
    // Интерактивная версия защиты
    virtual void act_player(std::vector<size_t>& alive_ids,
                            NightActions& night_actions,
                            std::vector<Shared_pointer<Player>>) override {
        std::cout << "Who do you want to protect?" << std::endl
                  << "Choose one of:" << std::endl;
        for (auto i : alive_ids) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        size_t choice;
        std::cin >> choice;
        night_actions.samurai_action = true;
        night_actions.samurai_choice = choice;
    }
};

// ============================================================================
// КЛАСС МАФИИ
// ============================================================================

class Mafia : public Player {
public:
    Mafia(size_t id_p) : Player(id_p) {
        team = "mafia";  // Команда мафии
        role = "mafia";  // Роль обычного мафиози
    }
    virtual ~Mafia() {}

    // AI-логика голосования мафии: голосует против не-мафиози
    virtual void vote_ai(std::vector<size_t>& alive_ids, size_t& value) override {
        simple_shuffle(alive_ids);
        size_t i = 0;
        // Ищем первого игрока, который не входит в известную мафию
        while (i < alive_ids.size()) {
            if (std::find(known_mafia.begin(), known_mafia.end(), alive_ids[i]) == known_mafia.end()) {
                value = alive_ids[i];
                return;
            }
            i++;
        }
        value = 0;
        return;
    }
    
    // AI-логика ночных действий: только босс мафии выбирает жертву
    virtual void act_ai(std::vector<size_t>& alive_ids,
                        NightActions& night_actions,
                        std::vector<Shared_pointer<Player>>) override {
        if (is_boss) {  // Только босс мафии совершает убийство
            simple_shuffle(alive_ids);
            size_t i = 0;
            // Ищем не-мафиози для убийства
            while (i < alive_ids.size()) {
                if (std::find(known_mafia.begin(), known_mafia.end(), alive_ids[i]) == known_mafia.end()) {
                    night_actions.killers[alive_ids[i]].push_back(id);
                    return;
                }
                i++;
            }
        }
    }
    
    // Интерактивная версия для реального игрока-мафии
    virtual void act_player(std::vector<size_t>& alive_ids,
                            NightActions& night_actions,
                            std::vector<Shared_pointer<Player>>) override {
        // Показываем известных мафиози
        std::cout << "Mafia members:" << std::endl;
        for (auto i : known_mafia) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        
        if (is_boss) {
            // Босс выбирает жертву
            std::cout << "You are a mafia boss. Who will the mafia kill on your orders?" << std::endl
                      << "Choose one of:" << std::endl;
            for (auto i : alive_ids) {
                std::cout << i << " ";
            }
            std::cout << std::endl;
            size_t choice;
            std::cin >> choice;
            night_actions.killers[choice].push_back(id);
        } else {
            // Обычный мафиозо не принимает решений
            std::cout << "You are not a mafia boss. Tonight you will not decide who the mafia will kill." << std::endl;
        }
    }
};

// ============================================================================
// КЛАСС БЫКА (УСИЛЕННЫЙ МАФИОЗИ)
// ============================================================================

class Bull : public Mafia {
public:
    Bull(size_t id_p) : Mafia(id_p) {
        role = "bull";  // Специальная роль в мафии
    }
    virtual ~Bull() {}
};

// ============================================================================
// КЛАСС МАНЬЯКА (ОДИНОЧКА)
// ============================================================================

class Maniac : public Player {
public:
    Maniac(size_t id_p) : Player(id_p) {
        team = "maniac";  // Собственная команда
        role = "maniac";  // Роль маньяка
    }
    virtual ~Maniac() {};

    // AI-логика голосования маньяка: голосует против любого другого игрока
    virtual void vote_ai(std::vector<size_t>& alive_ids, size_t& value) override {
        simple_shuffle(alive_ids);
        size_t i = 0;
        while (i < alive_ids.size()) {
            if (alive_ids[i] != id) {
                value = alive_ids[i];
                return;
            }
            i++;
        }
        value = 0;
        return;
    }
    
    // AI-логика ночных действий: убивает случайного игрока
    virtual void act_ai(std::vector<size_t>& alive_ids,
                        NightActions& night_actions,
                        std::vector<Shared_pointer<Player>>) override {
        simple_shuffle(alive_ids);
        size_t i = 0;
        while (i < alive_ids.size()) {
            if (alive_ids[i] != id) {  // Не может убить себя
                night_actions.killers[alive_ids[i]].push_back(id);
                return;
            }
            i++;
        }
    }
    
    // Интерактивная версия для реального игрока-маньяка
    virtual void act_player(std::vector<size_t>& alive_ids,
                            NightActions& night_actions,
                            std::vector<Shared_pointer<Player>>) override {
        std::cout << "Choose your victim:" << std::endl;
        for (auto i : alive_ids) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        size_t choice;
        std::cin >> choice;
        night_actions.killers[choice].push_back(id);
    }
};

// ============================================================================
// КОНЦЕПТ ДЛЯ ПРОВЕРКИ ТИПОВ ИГРОКОВ НА ЭТАПЕ КОМПИЛЯЦИИ
// ============================================================================

template<typename T>
concept PlayerConcept = requires(T player,
                                 std::vector<size_t> ids,
                                 size_t value,
                                 NightActions night_actions,
                                 std::vector<Shared_pointer<Player>> players) {
    // Проверяем, что тип T имеет метод vote с правильной сигнатурой
    { player.vote(ids, value) } -> std::same_as<Task>;
    // Проверяем, что тип T имеет метод act с правильной сигнатурой  
    { player.act(ids, night_actions, players) } -> std::same_as<Task>;
};

// ============================================================================
// КЛАСС ИГРЫ (УПРАВЛЯЕТ ВСЕМ ИГРОВЫМ ПРОЦЕССОМ)
// ============================================================================

template<PlayerConcept Player>
class Game {
public:
    Logger* logger;  // Логгер для записи событий игры
    std::vector<Shared_pointer<Player>> players {};  // Вектор всех игроков
    unsigned int players_num;     // Общее количество игроков
    unsigned int mafia_modifier;  // Модификатор для расчета количества мафии
    
    // Доступные роли для мирных жителей и мафии
    std::vector<std::string> civilian_roles {"commissar", "doctor", "journalist", "samurai"};
    std::vector<std::string> mafia_roles {"bull"};
    
    size_t samurai_id;  // ID самурая (для специальной логики)
    size_t bull_id;     // ID быка (для специальной логики)

    // Конструктор игры
    explicit Game(unsigned int players_num_, unsigned int mafia_modifier_ = 3) :
        players_num(players_num_),
        mafia_modifier(mafia_modifier_)
    {}

    // Вспомогательный метод для добавления случайных ролей
    void add_random_roles(
            std::vector<std::string> roles,
            size_t limit,
            std::string default_role,
            std::vector<std::string>& result) {
        std::mt19937 g(rand());
        size_t i = 0;
        std::shuffle(roles.begin(), roles.end(), g);  // Перемешиваем роли
        while (i < limit) {
            if (i < roles.size()) {
                result.push_back(roles[i]);  // Берем специальную роль
            } else {
                result.push_back(default_role);  // Или роль по умолчанию
            }
            i++;
        }
    }

    // Генерация случайного распределения ролей
    std::vector<std::string> get_random_roles() {
        unsigned int mafia_num = players_num / mafia_modifier;  // Расчет количества мафии
        std::vector<std::string> random_roles;

        // Добавляем роли мафии
        add_random_roles(mafia_roles, mafia_num, "mafia", random_roles);
        // Добавляем маньяка
        random_roles.push_back("maniac");
        // Добавляем роли мирных жителей
        add_random_roles(civilian_roles, players_num - mafia_num - 1, "civilian", random_roles);
        
        // Перемешиваем итоговый список ролей
        std::mt19937 g(rand());
        std::shuffle(random_roles.begin(), random_roles.end(), g);
        return random_roles;
    }

    // Инициализация игроков с заданными ролями
    void init_players(std::vector<std::string> roles) {
        players.clear();
        logger = new Logger{"init.log"};
        logger->log(Loglevel::INFO, "--- INIT ---");

        // Запрос у пользователя, хочет ли он играть
        std::cout << "Do you want to play? Select the number of the player (from 0 to " << roles.size() - 1
                  << ") you want to play or -1 if you don't want to." << std::endl;
        int choice;
        std::cin >> choice;
        
        std::vector<size_t> mafia_buf{};  // Временный буфер для ID мафиози
        
        // Создаем игроков согласно распределению ролей
        for (size_t i = 0; i < roles.size(); i++) {
            auto role = roles[i];
            if (role == "civilian") {
                logger->log(Loglevel::INFO,
                        TPrettyPrinter().f("Player ").f(i).f(" is civilian").Str());
                players.push_back(Shared_pointer<Player>(new Civilian{i}));
            } else if (role == "mafia") {
                logger->log(Loglevel::INFO,
                        TPrettyPrinter().f("Player ").f(i).f(" is mafia").Str());
                mafia_buf.push_back(i);
                players.push_back(Shared_pointer<Player>(new Mafia{i}));
            } else if (role == "maniac") {
                logger->log(Loglevel::INFO,
                        TPrettyPrinter().f("Player ").f(i).f(" is maniac").Str());
                players.push_back(Shared_pointer<Player>(new Maniac{i}));
            } else if (role == "bull") {
                logger->log(Loglevel::INFO,
                        TPrettyPrinter().f("Player ").f(i).f(" is bull").Str());
                players.push_back(Shared_pointer<Player>(new Bull{i}));
                mafia_buf.push_back(i);
                bull_id = i;
            } else if (role == "commissar") {
                logger->log(Loglevel::INFO,
                        TPrettyPrinter().f("Player ").f(i).f(" is commissar").Str());
                players.push_back(Shared_pointer<Player>(new Commissar{i}));
            } else if (role == "doctor") {
                logger->log(Loglevel::INFO,
                        TPrettyPrinter().f("Player ").f(i).f(" is doctor").Str());
                players.push_back(Shared_pointer<Player>(new Doctor{i}));
            } else if (role == "journalist") {
                logger->log(Loglevel::INFO,
                        TPrettyPrinter().f("Player ").f(i).f(" is journalist").Str());
                players.push_back(Shared_pointer<Player>(new Journalist{i}));
            } else if (role == "samurai") {
                logger->log(Loglevel::INFO,
                        TPrettyPrinter().f("Player ").f(i).f(" is samurai").Str());
                players.push_back(Shared_pointer<Player>(new Samurai{i}));
                samurai_id = i;
            }
            
            // Если игрок выбрал эту роль, помечаем как реального игрока
            if (std::cmp_equal(choice, i)) {
                players[i]->is_real_player = true;
                std::cout << "You are " << players[i]->role << "!" << std::endl;
                if (players[i]->role == "samurai") {
                    std::cout << "Wake up, Samurai! We have a city to burn!" << std::endl;
                }
            }
        }
        
        // Сообщаем мафиози друг о друге
        for (auto i : mafia_buf) {
            players[i]->known_mafia.insert(players[i]->known_mafia.end(), mafia_buf.begin(),
                    mafia_buf.end());
        }
        
        // Выбираем случайного босса мафии
        simple_shuffle(mafia_buf);
        players[mafia_buf[0]]->is_boss = true;
        delete logger;
    }

    // Перевыборы босса мафии (если предыдущий убит)
    void reelection_mafia_boss() {
        // Используем ranges для фильтрации живых мафиози
        auto mafia = players |
                     view::filter([](auto p) { return p->alive; }) |
                     view::filter([](auto p) { return p->team == "mafia"; });
        
        // Если есть живые мафиози, но нет босса - выбираем нового
        if (!mafia.empty() &&
                (mafia | view::filter([](auto p) { return p->is_boss; })).empty()) {
            std::vector<Shared_pointer<Player>> mafia_vec{mafia.begin(), mafia.end()};
            simple_shuffle(mafia_vec);
            mafia_vec[0]->is_boss = true;
        }
    }

    // Проверка текущего состояния игры
    std::string game_status() {
        // Фильтруем живых игроков с помощью ranges
        auto alives = players | view::filter([](auto p) { return p->alive; });
        
        if (alives.empty()) {
            // Все игроки мертвы - ничья
            return "draw";
        } else {
            // Фильтруем живых мафиози
            auto mafia = alives | view::filter([](auto p) { return p->team == "mafia"; });
            
            if (mafia.empty()) {
                // Мафия мертва
                auto maniac_alive = alives | view::filter([](auto p) { return p->team == "maniac"; });
                if (maniac_alive.empty()) {
                    // Маньяк тоже мертв - победа мирных
                    return "civilian";
                } else {
                    // Маньяк жив - проверяем условия победы маньяка
                    if (std::ranges::distance(alives) >= 3) {
                        return "continue";  // Игра продолжается
                    } else {
                        return "maniac";    // Победа маньяка
                    }
                }
            } else {
                // Мафия жива
                auto maniac_alive = alives | view::filter([](auto p) { return p->team == "maniac"; });
                if (maniac_alive.empty()) {
                    // Маньяк мертв - проверяем условия победы мафии
                    auto alives_num = std::ranges::distance(alives);
                    auto mafia_num = std::ranges::distance(mafia);
                    if (alives_num <= mafia_num * 2) {
                        return "mafia";     // Победа мафии
                    } else {
                        return "continue";  // Игра продолжается
                    }
                } else {
                    // Маньяк жив - игра продолжается
                    return "continue";
                }
            }
        }
    }

    // Главный игровой цикл (день-ночь-день-ночь...)
    void main_loop() {
        unsigned int day_number = 0;
        std::string cur_status = "";
        
        while (true) {
            // Создаем логгер для текущего дня
            logger = new Logger{"day_" + std::to_string(day_number) + ".log"};
            logger->log(Loglevel::INFO, "--- DAY " + std::to_string(day_number) + " ---");
            
            // Фаза дня: голосование
            day_vote();
            // Перевыборы босса мафии (если нужно)
            reelection_mafia_boss();
            // Проверяем состояние игры
            cur_status = game_status();
            if (cur_status != "continue") {
                delete logger;
                break;  // Игра завершена
            }
            
            // Фаза ночи: действия игроков
            night_act();
            reelection_mafia_boss();
            cur_status = game_status();
            if (cur_status != "continue") {
                delete logger;
                break;  // Игра завершена
            }
            
            day_number++;
            delete logger;
        }
        
        // Записываем финальный результат
        logger = new Logger{"result.log"};
        if (cur_status == "draw") {
            logger->log(Loglevel::INFO, "No one survived the brutal shootouts and nighttime murders... The city died out...");
            logger->log(Loglevel::INFO, "DRAW!");
            logger->log(Loglevel::INFO, "Alives: ---");
        } else if (cur_status == "mafia") {
            logger->log(Loglevel::INFO, "The mafia has taken over this city and no one can stop them anymore. The mafia never dies!");
            logger->log(Loglevel::INFO, "MAFIA WIN");
        } else if (cur_status == "maniac") {
            logger->log(Loglevel::INFO, "Neither the mafia, nor the peaceful civilian, nor the sheriff could stop the crazy loner in the night...");
            logger->log(Loglevel::INFO, "MANIAC WINS");
        } else if (cur_status == "civilian") {
            logger->log(Loglevel::INFO, "The city sleeps peacefully. The citizens united and fought back against the mafia and the maniac.");
            logger->log(Loglevel::INFO, "CIVILIANS WIN");
        }
        
        // Записываем выживших игроков
        logger->log(Loglevel::INFO, "Alives:");
        for (const auto& player : players) {
            if (player->alive) {
                logger->log(Loglevel::INFO, TPrettyPrinter().f("Player ").f(player->id).f(" - ").f(player->role).Str());
            }
        }
        delete logger;
    }

    // Фаза дневного голосования
    void day_vote() {
        // Используем ranges для получения ID живых игроков
        auto alives = players | view::filter([](auto p) { return p->alive; });
        auto alives_ids_rng = alives | view::transform([](auto p) { return p->id; });
        std::vector<size_t> alives_ids{alives_ids_rng.begin(), alives_ids_rng.end()};
        
        // Создаем карту для подсчета голосов
        std::map<size_t, unsigned int> votes{};
        for (auto id: alives_ids) {
            votes[id] = 0;  // Инициализируем нулями
        }

        // Перемешиваем порядок голосования
        std::vector<Shared_pointer<Player>> sh_alives{alives.begin(), alives.end()};
        simple_shuffle(sh_alives);
        
        // Каждый игрок голосует
        for (const auto& player : sh_alives) {
            size_t value = 0;
            player->vote(alives_ids, value);  // Вызываем корутину голосования
            votes[value]++;  // Увеличиваем счетчик голосов
            logger->log(Loglevel::INFO,
                    TPrettyPrinter().f("Player ").f(player->id).f(" voted for player ").f(value).Str());
        }

        // Находим игрока с максимальным количеством голосов
        auto key_val = std::max_element(votes.begin(), votes.end(),
                [](const std::pair<int, int>& p1, const std::pair<int, int>& p2) {
                    return p1.second < p2.second;
                });
        
        // "Казним" игрока с наибольшим количеством голосов
        players[key_val->first]->alive = false;
        logger->log(Loglevel::INFO,
                TPrettyPrinter().f("Player ").f(key_val->first).f(" was executed by order of the city.").Str());
    }

    // Фаза ночных действий
    void night_act() {
        // Получаем ID живых игроков с помощью ranges
        auto alives = players | view::filter([](auto p) { return p->alive; });
        auto alives_ids_rng = alives | view::transform([](auto p) { return p->id; });
        std::vector<size_t> alives_ids{alives_ids_rng.begin(), alives_ids_rng.end()};
        
        // Создаем и сбрасываем структуру ночных действий
        NightActions night_actions{players_num};
        night_actions.reset();

        // Каждый игрок совершает свое ночное действие
        for (const auto& player : alives) {
            player->act(alives_ids, night_actions, players);  // Вызываем корутину действия
        }

        // Специальная логика для Быка: защита от маньяка
        for (size_t i = 0; i < night_actions.killers[bull_id].size(); i++) {
            auto killer_id = night_actions.killers[bull_id][i];
            if (players[killer_id]->role == "maniac") {
                night_actions.killers[bull_id].erase(night_actions.killers[bull_id].begin() + i);
                break;
            }
        }
        
        // Обрабатываем специальные действия и логируем их
        
        if (night_actions.commissar_action) {
            logger->log(Loglevel::INFO, TPrettyPrinter().f("Commissar checked player ").f(night_actions.commissar_choice).f(
                        ". He was a ").f(players[night_actions.commissar_choice]->role).Str());
        }
        
        if (night_actions.doctors_action) {
            logger->log(Loglevel::INFO, TPrettyPrinter().f("Doctor healed player ").f(night_actions.doctors_choice).Str());
            // Лечение снимает все атаки с игрока
            night_actions.killers[night_actions.doctors_choice].clear();
        }
        
        if (night_actions.journalist_action) {
            logger->log(Loglevel::INFO, TPrettyPrinter().f("Journalist checked players ").f(
                        night_actions.journalist_choice.first).f(" and ").f(
                        night_actions.journalist_choice.second).Str());
        }
        
        if (night_actions.samurai_action) {
            logger->log(Loglevel::INFO, TPrettyPrinter().f("Samurai protected player ").f(night_actions.samurai_choice).Str());
            auto& killers_list = night_actions.killers[night_actions.samurai_choice];
            if (!killers_list.empty()) {
                // Самурай контратакует первого атакующего
                simple_shuffle(killers_list);
                night_actions.killers[killers_list[0]].push_back(samurai_id);
                // Самурай также получает урон (жертвует собой)
                night_actions.killers[samurai_id].push_back(samurai_id);
                // Защищенный игрок не получает урона
                night_actions.killers[night_actions.samurai_choice].clear();
            }
        }

        // Обрабатываем все убийства
        for (size_t i = 0; i < players_num; i++) {
            if (!night_actions.killers[i].empty()) {
                // Формируем сообщение о убийстве
                auto log_message = TPrettyPrinter().f("Player ").f(i).f(" was killed by ").Str();
                for (size_t j = 0; j < night_actions.killers[i].size(); j++) {
                    log_message += players[night_actions.killers[i][j]]->role;
                    log_message += (j == night_actions.killers[i].size() - 1) ? "" : ", ";
                }
                // Убиваем игрока
                players[i]->alive = false;
                logger->log(Loglevel::INFO, log_message);
            }
        }
    }
};

// ============================================================================
// ГЛАВНАЯ ФУНКЦИЯ ПРОГРАММЫ
// ============================================================================

int main(void) {
    // Цикл для многократного запуска игры (для тестирования)
    for (int i = 0; i < 200; i++) {
        std::srand(5);  // Фиксированный seed для воспроизводимости
        std::cout << "========== SRAND = " << i << " ==========" << std::endl;
        
        // Создаем игру на 10 игроков
        auto game = Game<Player>(10);
        // Генерируем случайные роли
        auto roles = game.get_random_roles();
        // Инициализируем игроков
        game.init_players(roles);
        // Запускаем главный игровой цикл
        game.main_loop();
        break;  // Прерываем после первой игры (для демонстрации)
    }
    return 0;
}
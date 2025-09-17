// roles.h
#pragma once
#include "player.h"

class Citizen : public Player
{
public:
    Citizen(int id, std::string name, bool human = false, std::function<int()> choice = nullptr)
        : Player(id, std::move(name), Role::CITIZEN, human, choice) {}

    Task performNightAction(const std::vector<MySharedPtr<Player>> &players) override
    {
        // Мирные жители не действуют ночью
        co_return;
    }

    Task<int> voteForExecution(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            co_return co_await getHumanChoice();
        }
        co_return chooseRandomPlayer(players);
    }
};

class Mafia : public Player
{
public:
    Mafia(int id, std::string name, bool human = false, std::function<int()> choice = nullptr)
        : Player(id, std::move(name), Role::MAFIA, human, choice) {}

    Task performNightAction(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            int target_id = co_await getHumanChoice();
            if (target_id != -1)
                killPlayer(players, target_id);
        }
        else
        {
            int target_id = chooseKillTarget(players);
            if (target_id != -1)
                killPlayer(players, target_id);
        }
        co_return;
    }

    Task<int> voteForExecution(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            co_return co_await getHumanChoice();
        }
        co_return chooseRandomPlayer(players);
    }

private:
    int chooseKillTarget(const std::vector<MySharedPtr<Player>> &players)
    {
        std::vector<int> valid_targets;
        for (const auto &player : players)
        {
            if (player->isAlive() && player->getRole() != Role::MAFIA && !player->isProtected())
            {
                valid_targets.push_back(player->getId());
            }
        }
        if (valid_targets.empty())
            return -1;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, valid_targets.size() - 1);
        return valid_targets[dis(gen)];
    }

    void killPlayer(const std::vector<MySharedPtr<Player>> &players, int target_id)
    {
        for (const auto &player : players)
        {
            if (player->getId() == target_id)
            {
                player->kill();
                std::cout << name << " (Mafia) killed " << player->getName() << std::endl;
                break;
            }
        }
    }
};

class Commissar : public Player
{
public:
    Commissar(int id, std::string name, bool human = false, std::function<int()> choice = nullptr)
        : Player(id, std::move(name), Role::COMMISSAR, human, choice) {}

    Task performNightAction(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            int target_id = co_await getHumanChoice();
            if (target_id != -1)
                checkPlayer(players, target_id);
        }
        else
        {
            int target_id = chooseCheckTarget(players);
            if (target_id != -1)
                checkPlayer(players, target_id);
        }
        co_return;
    }

    Task<int> voteForExecution(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            co_return co_await getHumanChoice();
        }
        co_return chooseRandomPlayer(players);
    }

private:
    int chooseCheckTarget(const std::vector<MySharedPtr<Player>> &players)
    {
        return chooseRandomPlayer(players); // Простая логика для бота
    }

    void checkPlayer(const std::vector<MySharedPtr<Player>> &players, int target_id)
    {
        for (const auto &player : players)
        {
            if (player->getId() == target_id)
            {
                std::cout << name << " (Commissar) checked " << player->getName()
                          << " - Role: " << roleToString(player->getRole()) << std::endl;
                break;
            }
        }
    }
};

class Doctor : public Player
{
public:
    Doctor(int id, std::string name, bool human = false, std::function<int()> choice = nullptr)
        : Player(id, std::move(name), Role::DOCTOR, human, choice) {}

    Task performNightAction(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            int target_id = co_await getHumanChoice();
            if (target_id != -1)
                protectPlayer(players, target_id);
        }
        else
        {
            int target_id = chooseProtectTarget(players);
            if (target_id != -1)
                protectPlayer(players, target_id);
        }
        co_return;
    }

    Task<int> voteForExecution(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            co_return co_await getHumanChoice();
        }
        co_return chooseRandomPlayer(players);
    }

private:
    int chooseProtectTarget(const std::vector<MySharedPtr<Player>> &players)
    {
        // Доктор может защитить любого живого игрока, включая себя
        std::vector<int> valid_targets;
        for (const auto &player : players)
        {
            if (player->isAlive())
            {
                valid_targets.push_back(player->getId());
            }
        }
        if (valid_targets.empty())
            return -1;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, valid_targets.size() - 1);
        return valid_targets[dis(gen)];
    }

    void protectPlayer(const std::vector<MySharedPtr<Player>> &players, int target_id)
    {
        for (const auto &player : players)
        {
            if (player->getId() == target_id)
            {
                player->setProtected(true);
                std::cout << name << " (Doctor) protected " << player->getName() << std::endl;
                break;
            }
        }
    }
};

class Maniac : public Player
{
public:
    Maniac(int id, std::string name, bool human = false, std::function<int()> choice = nullptr)
        : Player(id, std::move(name), Role::MANIAC, human, choice) {}

    Task performNightAction(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            int target_id = co_await getHumanChoice();
            if (target_id != -1)
                killPlayer(players, target_id);
        }
        else
        {
            int target_id = chooseKillTarget(players);
            if (target_id != -1)
                killPlayer(players, target_id);
        }
        co_return;
    }

    Task<int> voteForExecution(const std::vector<MySharedPtr<Player>> &players) override
    {
        if (is_human)
        {
            co_return co_await getHumanChoice();
        }
        co_return chooseRandomPlayer(players);
    }

private:
    int chooseKillTarget(const std::vector<MySharedPtr<Player>> &players)
    {
        std::vector<int> valid_targets;
        for (const auto &player : players)
        {
            if (player->isAlive() && player->getId() != id && !player->isProtected())
            {
                valid_targets.push_back(player->getId());
            }
        }
        if (valid_targets.empty())
            return -1;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, valid_targets.size() - 1);
        return valid_targets[dis(gen)];
    }

    void killPlayer(const std::vector<MySharedPtr<Player>> &players, int target_id)
    {
        for (const auto &player : players)
        {
            if (player->getId() == target_id)
            {
                player->kill();
                std::cout << name << " (Maniac) killed " << player->getName() << std::endl;
                break;
            }
        }
    }
};
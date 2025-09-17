#pragma once
#include <vector>
#include <string>
#include <random>
#include <ranges>
#include <iostream>
#include "shr_ptr.h"
#include "coroutines.h"
#include "concepts.h"
#include "shuffle.h"

enum Role
{
    MAFIA,
    CITIZEN,
    COMMISSAR,
    DOCTOR,
    MANIAC
}; // Возможные игровые персонажи

enum Team
{
    CIVILIAN,
    MAFIOZI,
    CRAZYMAN
}; // Фракции героев для определения стороны победителя

std::string roleToString(Role role)
{
    /*
    Преобразование названия роли в строку
    */

    switch (role)
    {
    case MAFIA:
        return "Mafia";
    case CITIZEN:
        return "Citizen";
    case COMMISSAR:
        return "Commissar";
    case DOCTOR:
        return "Doctor";
    case MANIAC:
        return "Maniac";
    default:
        return "Unknown";
    }
}

std::string teamsToString(Team team)
{
    /*
    Преобразование названия команды в строку
    */

    switch (team)
    {
    case CIVILIAN:
        return "Civilian";
    case MAFIOZI:
        return "Mafia";
    case CRAZYMAN:
        return "Maniac";
    }
}

class Player
{
protected:
    int id;
    std::string name;
    Role role;
    bool alive = true;
    bool protected_tonight = false;
    bool is_human = false;
    std::function<int()> human_choice;

public:
    Player(int id, std::string name, Role role, bool human = false,
           std::function<int()> choice = nullptr)
        : id(id), name(std::move(name)), role(role),
          is_human(human), human_choice(choice) {}

    virtual ~Player() = default;

    int getId() const { return id; }
    std::string getName() const { return name; }
    Role getRole() const { return role; }
    bool isAlive() const { return alive; }
    bool isProtected() const { return protected_tonight; }
    bool isHuman() const { return is_human; }

    void setProtected(bool protect) { protected_tonight = protect; }
    void kill() { alive = false; }
    void revive()
    {
        alive = true;
        protected_tonight = false;
    }

    virtual Task performNightAction(const std::vector<MySharedPtr<Player>> &players) = 0;
    virtual Task<int> voteForExecution(const std::vector<MySharedPtr<Player>> &players) = 0;

    std::string getDescription() const
    {
        return name + " (" + roleToString(role) + (is_human ? ", Human" : ", Bot") + ")";
    }

protected:
    int chooseRandomPlayer(const std::vector<MySharedPtr<Player>> &players)
    {
        std::vector<int> alive_ids;
        for (const auto &player : players)
        {
            if (player->isAlive() && player->getId() != id)
            {
                alive_ids.push_back(player->getId());
            }
        }

        if (alive_ids.empty())
            return -1;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, alive_ids.size() - 1);
        return alive_ids[dis(gen)];
    }

    Task<int> getHumanChoice()
    {
        if (is_human && human_choice)
        {
            ChoiceAwaiter choice{human_choice};
            co_return co_await choice;
        }
        co_return -1;
    }
};
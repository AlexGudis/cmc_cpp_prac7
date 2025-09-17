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

enum Role { MAFIA, CITIZEN, COMMISSAR, DOCTOR, MANIAC }; // Возможные игровые персонажи

enum Team { CIVILIAN, MAFIOZI, CRAZYMAN }; // Фракции героев для определения стороны победителя
 
std::string roleToString(Role role) { 
    /*
    Преобразование названия роли в строку
    */

    switch (role) {
        case MAFIA: return "Mafia";
        case CITIZEN: return "Citizen";
        case COMMISSAR: return "Commissar";
        case DOCTOR: return "Doctor";
        case MANIAC: return "Maniac";
        default: return "Unknown";
    }
}


std::string teamsToString(Team team) {
    /*
    Преобразование названия команды в строку
    */

    switch (team) {
        case CIVILIAN: return "Civilian";
        case MAFIOZI: return "Mafia";
        case CRAZYMAN: return "Maniac";
    }
}


class Player {
protected:
    int id; // Номер игрока
    std::string name; // Имя игрока
    Role role; // Роль игрока
    Team team; // Команда игрока

    
    bool alive = true; // Живой?
    bool protected_tonight = false; // Возможно ли убить эту роль сегодня? (На случай действий доктора и других ролей)
    bool is_human = false; // Будем ли управлять этой ролью лично
    
public:
    Player(int id, std::string name, Role role, bool human = false)
            : id(id), name(std::move(name)), role(role), is_human(human) {}
        
    virtual ~Player() = default;
        
    int getId() const { return id; }
    std::string getName() const { return name; }
    Role getRole() const { return role; }
    Team getTeam() const {return team; }
    bool isAlive() const { return alive; }
    bool isProtected() const { return protected_tonight; }
    bool isHuman() const { return is_human; }
        
    void setProtected(bool protect) { protected_tonight = protect; }
    void kill() { alive = false; }
        
    virtual Task nightAction(const std::vector<MySharedPtr<Player>>& players) = 0;
    virtual Task<int> vote(const std::vector<MySharedPtr<Player>>& players) = 0;
        
    std::string getDescription() const {
            return name + " (" + roleToString(role) + (is_human ? ", Human" : ", Bot") + ")";
        }
    
protected:
    int chooseRandom(const std::vector<MySharedPtr<Player>>& players) {

        // TODO: make it simplier
        auto alive_players = players | std::views::filter([](const auto& p) { 
            return p->isAlive(); 
        });
            
        if (alive_players.empty()) return -1;
            
        std::vector<int> valid_ids;
        for (const auto& player : alive_players) {
            valid_ids.push_back(player->getId());
        }
            
        simple_shuffle(valid_ids);
        return valid_ids[0];
    }
};
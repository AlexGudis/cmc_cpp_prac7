#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include <ranges>
#include "roles.h"

class GameMaster
{
private:
    std::vector<MySharedPtr<Player>> players;
    int total_players;
    bool verbose;
    std::function<int()> global_choice_callback;

public:
    GameMaster(int n, bool verb, std::function<int()> callback = nullptr)
        : total_players(n), verbose(verb), global_choice_callback(callback) {}

    void initializeGame(int human_player_id = -1, Role human_role = Role::CITIZEN)
    {
        // Распределение ролей
        int mafia_count = std::max(1, total_players / 3);
        std::vector<Role> roles(total_players, Role::CITIZEN);

        for (int i = 0; i < mafia_count; ++i)
            roles[i] = Role::MAFIA;
        if (total_players > 3)
            roles[3] = Role::COMMISSAR;
        if (total_players > 4)
            roles[4] = Role::DOCTOR;
        if (total_players > 5)
            roles[5] = Role::MANIAC;

        std::shuffle(roles.begin(), roles.end(), std::mt19937{std::random_device{}()});

        // Создаем игроков с конкретными классами ролей
        for (int i = 0; i < total_players; ++i)
        {
            std::string name = "Player_" + std::to_string(i + 1);
            bool is_human = (i == human_player_id);

            switch (roles[i])
            {
            case Role::CITIZEN:
                players.push_back(MySharedPtr<Player>(
                    new Citizen(i, name, is_human, global_choice_callback)));
                break;
            case Role::MAFIA:
                players.push_back(MySharedPtr<Player>(
                    new Mafia(i, name, is_human, global_choice_callback)));
                break;
            case Role::COMMISSAR:
                players.push_back(MySharedPtr<Player>(
                    new Commissar(i, name, is_human, global_choice_callback)));
                break;
            case Role::DOCTOR:
                players.push_back(MySharedPtr<Player>(
                    new Doctor(i, name, is_human, global_choice_callback)));
                break;
            case Role::MANIAC:
                players.push_back(MySharedPtr<Player>(
                    new Maniac(i, name, is_human, global_choice_callback)));
                break;
            }
        }
    }

    Task playGame()
    {
        std::cout << "=== MAFIA GAME STARTED ===\n";
        printPlayers();

        for (int day = 1; !isGameOver(); ++day)
        {
            std::cout << "\n=== DAY " << day << " ===\n";
            co_await dayPhase();

            if (isGameOver())
                break;

            std::cout << "\n=== NIGHT " << day << " ===\n";
            co_await nightPhase();
        }

        announceWinner();
        co_return;
    }

    Task dayPhase()
    {
        std::map<int, int> votes;

        for (const auto &player : players)
        {
            if (player->isAlive())
            {
                ValueTask<int> vote_task = player->voteForExecution(players);
                int vote = vote_task.get(); // Получаем результат
                if (vote != -1)
                {
                    votes[vote]++;
                    if (verbose)
                    {
                        std::cout << player->getName() << " voted for "
                                  << getPlayerName(vote) << "\n";
                    }
                }
            }
        }

        if (!votes.empty())
        {
            auto [target_id, count] = *std::ranges::max_element(votes,
                                                                [](auto a, auto b)
                                                                { return a.second < b.second; });

            if (count > 0)
            {
                players[target_id]->kill();
                std::cout << players[target_id]->getName() << " was executed!\n";
            }
        }
        co_return;
    }

    Task nightPhase()
    {
        // Сброс защиты
        for (auto &player : players)
        {
            player->setProtected(false);
        }

        // Ночные действия по ролям
        for (const auto &player : players)
        {
            if (player->isAlive() && player->getRole() != Role::CITIZEN)
            {
                co_await player->performNightAction(players);
            }
        }
        co_return;
    }

    bool isGameOver()
    {
        int mafia = 0, citizens = 0;
        for (const auto &player : players)
        {
            if (player->isAlive())
            {
                (player->getRole() == Role::MAFIA ? mafia : citizens)++;
            }
        }
        return mafia == 0 || mafia >= citizens;
    }

    void printPlayers()
    {
        std::cout << "Players:\n";
        for (const auto &player : players)
        {
            std::cout << "  " << player->getDescription() << "\n";
        }
    }

    void announceWinner()
    {
        int mafia = 0;
        for (const auto &player : players)
        {
            if (player->isAlive() && player->getRole() == Role::MAFIA)
            {
                mafia++;
            }
        }

        std::cout << "\n=== GAME OVER ===\n";
        std::cout << (mafia == 0 ? "CITIZENS" : "MAFIA") << " WIN!\n";

        std::cout << "\nFinal status:\n";
        for (const auto &player : players)
        {
            std::cout << "  " << player->getName() << ": "
                      << (player->isAlive() ? "ALIVE" : "DEAD") << "\n";
        }
    }

    std::string getPlayerName(int id)
    {
        for (const auto &player : players)
        {
            if (player->getId() == id)
                return player->getName();
        }
        return "Unknown";
    }
};
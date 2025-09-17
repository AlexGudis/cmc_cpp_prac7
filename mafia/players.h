#pragma once
#include <vector>
#include <string>
#include <random>
#include <ranges>
#include <iostream>
#include "shr_ptr.h"
#include "coroutines.h"
#include "concepts.h"

enum Role { MAFIA, CITIZEN, COMMISSAR, DOCTOR, MANIAC };

std::string roleToString(Role role) {
    switch (role) {
        case MAFIA: return "Mafia";
        case CITIZEN: return "Citizen";
        case COMMISSAR: return "Commissar";
        case DOCTOR: return "Doctor";
        case MANIAC: return "Maniac";
        default: return "Unknown";
    }
}
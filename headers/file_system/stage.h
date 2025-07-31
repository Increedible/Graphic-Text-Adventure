#pragma once
#include <string>
#include <optional>
#include "../misc/utilities.h"

struct Stage {
    int id;
    std::string name;
    std::string dialogue, conclusion;
    int directions[4];
    image<16> vis;
    image<6> legend;
    int needcoins, coinreward;
    std::optional<int> shop, minigame;
    std::optional<std::pair<int, std::string>> encounter; // opponent, dialogue
    std::vector<std::pair<int, double>> possibleEncounters; // opponent, probability
};
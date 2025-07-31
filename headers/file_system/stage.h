#pragma once
#include <string>
#include <array>
#include <vector>
#include <optional>
#include "../misc/utilities.h"
#include "../misc/io_utils.h"

struct Stage {
    int id;
    StyleString name;
    StyleString dialogue, conclusion;
    std::array<int,4> directions;
    image<16> vis;
    image<6> legend;
    int needcoins, coinreward;
    std::optional<int> shop, minigame;
    std::optional<std::pair<int, StyleString>> encounter; // opponent, dialogue
    std::vector<std::pair<int, double>> possibleEncounters; // opponent, probability
};
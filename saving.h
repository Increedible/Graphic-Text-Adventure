#pragma once
#include <string>
#include <map>
#include "inventory.h"

constexpr int defCoins = 10;
constexpr int defStamina = 10;
constexpr int defSanity = 10;
constexpr int defHealth = 100;
constexpr int defPlayerDmg = 50;
constexpr int defPlayerRes = 0;

extern const std::map<int, int> defStageEncounters;

struct saveState {
    int stage, coins, stamina, sanity, health, playerDmg, playerRes;
    std::map<int, int> stageEncounters;
    Inventory inventory;

    void reset();
    saveState();
};

int stageOfSave(std::string index);
std::pair<bool, saveState> loadGame(std::string index);
bool saveGame(std::string index, saveState& s, int inventorymax);

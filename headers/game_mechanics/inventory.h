#pragma once
#include <vector>
#include "items.h"
#include "../system_io/input.h"

struct saveState;

struct Inventory {
    std::vector<int> inventory;
    int max_elements = 10;
    bool addToInventory(int item);
    bool inventoryHas(int item);
    void interactInventory(saveState &cursave, MyIO &io);
    void remove(int index);
    size_t size();
    int& operator[](int index);
};
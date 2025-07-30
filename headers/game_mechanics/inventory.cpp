#include <map>
#include <string>
#include "inventory.h"
#include "items.h"
#include "../file_system/saving.h"
#include "../misc/io_utils.h"

bool Inventory::addToInventory(int item) {
    if (inventory.size() >= max_elements)
        return false;
    inventory.push_back(item);
    return true;
}

bool Inventory::inventoryHas(int item) {
    for (int i:inventory) {
        if (i==item) return true;
    }
    return false;
}

void Inventory::interactInventory(saveState &cursave, my_io &io) {
    bool exit = false;
    while (!exit) {
        std::vector<Option> options;
        if (inventory.size() <= 0)
            typeOut(io, "\nYour inventory is empty.");
        else {
            typeOut(io, "\nYour items:");
            for (int i = 0; i < (int)inventory.size(); i++)
                options.push_back({std::to_string(i + 1) + ".\t" + items[inventory[i]].name, i});
        }
        options.push_back({"Exit", -1, Color::Blue});
        int choice = optionsNav(io, options, "Inv");
        if (choice == -1)
            exit = true;
        else {
            int size = inventory.size();
            item &it = items[inventory[choice]];
            typeOut(io, "\n" + colored(it.name, Color::Magenta));
            std::vector<Option> options2;
            options2.push_back({"Drop", 0, it.isdroppable?Color::Green:Color::Red});
            options2.push_back({"Use", 1, it.isusable?Color::Green:Color::Red});
            options2.push_back({"Eat", 2, it.healthrestore?Color::Green:Color::Red});
            options2.push_back({"Exit", 3});
            int choice2 = optionsNav(io, options2, "Item");
            if (choice2 == 0) {
                if (it.isdroppable) {
                    remove(choice);
                    typeOut(io, "Succesfully dropped " + colored(it.name, Color::Magenta) + "! [Press Enter]");
                } else {
                    typeOut(io, "This item cannot be dropped! [Press Enter]");
                }
                wait_enter(io);
            } else if (choice2 == 1) {
                if (it.isusable) {
                    // use the item, somehow
                    remove(choice);
                    typeOut(io, "Succesfully used " + colored(it.name, Color::Magenta) + "! [Press Enter]");
                }
                else
                    typeOut(io, "This item cannot be used! [Press Enter]");
                wait_enter(io);
            } else if (choice2 == 2) {
                if (it.healthrestore>0) {
                    remove(choice);
                    if (cursave.health + it.healthrestore > defHealth) {
                        cursave.health = defHealth;
                        typeOut(io, "You eat the " + colored(it.name, Color::Magenta) + ".");
                        typeOut(io, "Your health was set back to " + colored(std::to_string(cursave.health), Color::Red) + ". [Press Enter]");
                    }
                    else {
                        cursave.health += it.healthrestore;
                        typeOut(io, "You eat the " + colored(it.name, Color::Magenta) + ".");
                        typeOut(io, "Your health was increased and now is " + colored(std::to_string(cursave.health), Color::Red) + ". [Press Enter]");
                    }
                } else {
                    typeOut(io, "This item cannot be eaten! [Press Enter]");
                }
                wait_enter(io);
            }
            // do nothing if choice2 = exit
        }
    }
}

size_t Inventory::size(){
    return inventory.size();
}

int& Inventory::operator[](int index){
    return inventory[index];
}

void Inventory::remove(int index) {
    inventory.erase(inventory.begin() + index);
}
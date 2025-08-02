#pragma once
#include <vector>
#include <iostream>
#include "items.h"
#include "../system_io/output.h"
#include "../file_system/saving.h"
#include "../misc/io_utils.h"

struct Purchasable {
    int itemid, buyprice, stock;
};

struct Shop {
    std::vector<int> availableitems;
    std::vector<int> itembuyprices;
    std::vector<int> itemstock;
    std::vector<int> itemsellprices;
    Shop(const std::vector<Purchasable>& buylist, const std::vector<std::pair<int, int>>& selllist) {
        for (const auto& i : buylist) {
            availableitems.push_back(i.itemid);
            itembuyprices.push_back(i.buyprice);
            itemstock.push_back(i.stock);
        }
        itemsellprices.assign(6, 0); // THIS HAS TO BE CHANGED
        for (const auto& i : selllist) {
            itemsellprices[i.first] = i.second;
        }
    };
    Shop(){};
};

void shop(MyIO &io, saveState& cursave, Shop& curshop) {
    bool exit = false;
    while (!exit) {
        typeOut(io, "\nCoins: " + colored(std::to_string(cursave.coins), Color::Yellow));
        std::cout << "\tItem\t\tPrice\tStock\n";
        std::vector<Option> options;
        for (size_t i = 0; i < curshop.availableitems.size(); i++){
            options.push_back(
                Option(items[curshop.availableitems[i]].name + "\t" +
                    std::to_string(curshop.itembuyprices[i]) + "\t" +
                    std::to_string(curshop.itemstock[i]),i
            ));
        }
        options.push_back(Option("Sell item(s)", -1, Color::Blue));
        options.push_back(Option("View inventory", -2, Color::Blue));
        options.push_back(Option("Exit", -3, Color::Blue));
        int choice = optionsNav(io, options, "Shop");
        if (choice == -1){
            // sell
            bool exit2 = false;
            while (!exit2) {
                std::vector<Option> options2;
                if (cursave.inventory.inventory.size() <= 0)
                    typeOut(io, "\nYour inventory is empty.");
                else {
                    typeOut(io, "\nYour items:");
                    for (int i = 0; i < (int)cursave.inventory.inventory.size(); i++){

                        options2.push_back({std::to_string(i + 1) + ".\t" + items[cursave.inventory.inventory[i]].name, i});
                    }
                }
                options2.push_back({"Exit", -1, Color::Blue});
                int choice2 = optionsNav(io, options2, "Sell");
                if (choice2 == -1){
                    exit2 = true;
                }
                else {
                    int item_id = cursave.inventory[choice2];
                    int sell_price = curshop.itemsellprices[item_id];
                    cursave.coins += sell_price;
                    cursave.inventory.remove(item_id);
                    typeOut(io, "You went ahead and gained " + colored(std::to_string(sell_price), Color::Yellow) + ((sell_price<=1) ? "coin" : "coin(s)") + "for selling your " + colored(items[item_id].name, Color::Magenta) + ".\n");
                     
                }
            }
        }
        else if (choice == -2) {
            cursave.inventory.interactInventory(cursave, io);
        }
        else if (choice == -3) {
            exit = true;
        }
        else { 
            if (curshop.itemstock[choice] > 0) {
                if (curshop.itembuyprices[choice] <= cursave.coins) {
                    if (cursave.inventory.addToInventory(curshop.availableitems[choice]) == true) {
                        cursave.coins -= curshop.itembuyprices[choice];
                        curshop.itemstock[choice] -= 1;
                        typeOut(io, "\nYou went ahead and bought a " + colored(items[curshop.availableitems[choice]].name, Color::Magenta) + ", and paid the shop owner " + colored(std::to_string(curshop.itembuyprices[choice]), Color::Yellow) + " coins.\n");
                    } else {
                        typeOut(io, "You wanted to buy a " + colored(items[curshop.availableitems[choice]].name, Color::Magenta) + ", but your inventory was full!\n");
                    }
                } else {
                    typeOut(io, "You wanted to buy a " + colored(items[curshop.availableitems[choice]].name, Color::Magenta) + ", but you didn't have enough coins!\n");
                }
            } else {
                typeOut(io, "You wanted to buy a " + colored(items[curshop.availableitems[choice]].name, Color::Magenta) + ", but it was out of stock!\n");
            }
        }
    }
    typeOut(io, "You exit the shop menu.\n");
}


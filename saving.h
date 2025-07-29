#include <string>
#include <fstream>
#include <vector>
struct saveState {
    int stage, coins, stamina, sanity, health, playerDmg, playerRes;
};

int stageOfSave(std::string index) {
    std::fstream f("savefile" + index + ".txt", std::ios::in);
    std::string line;
    if (f.is_open()) {
        getline(f, line);
        f.close();
        return std::stoi(line);
    }
    else
        return 0;
}

bool loadGame(std::string index) {
    std::fstream f("savefile" + index + ".txt", std::ios::in);
    std::vector<std::string> lines;
    std::string line;
    if (f.is_open()) {
        std::getline(f, line);
        while (f) {
            lines.push_back(line);
            std::getline(f, line);
        }
        f.close();
        stage = stoi(lines[0]);
        coins = stoi(lines[1]);
        stamina = stoi(lines[2]);
        sanity = stoi(lines[3]);
        health = stoi(lines[4]);
        playerDmg = stoi(lines[5]);
        playerRes = stoi(lines[6]);
        int i = 7;
        for (auto& x : stageEncounters) {
            x.second = stoi(lines[i]);
            i++;
        }
        for (int j = i; i < j+10; i++)
            if (lines[i] != "0") {
                inventory.push_back(stoi(lines[i]));
            }
        return true;
    }
    else
        return false;
}

bool saveGame(string index) {
    fstream f("savefile" + index + "temp.txt", ios::out);
    string fileData = "";
    fileData += to_string(stage) + "\n";
    fileData += to_string(coins) + "\n";
    fileData += to_string(stamina) + "\n";
    fileData += to_string(sanity) + "\n";
    fileData += to_string(health) + "\n";
    fileData += to_string(playerDmg) + "\n";
    fileData += to_string(playerRes) + "\n";
    for (auto const& x : stageEncounters)
        fileData += to_string(x.second) + "\n";
    for (int i = 0; i < (int)inventory.size(); i++)
        fileData += to_string(inventory[i]) + "\n";
    for (int i = 0; i < (int)(inventorymax - inventory.size()); i++)
        fileData += "0\n";
    if (f.is_open()) {
        for (int i = 0; i < (int)fileData.size(); i++) {
            f.put(fileData[i]);
        }
        f.close();
        int returnint;
        if (index == "1") {
            remove("savefile1.txt");
            returnint = rename("savefile1temp.txt", "savefile1.txt");
        }
        if (index == "2") {
            remove("savefile2.txt");
            returnint = rename("savefile2temp.txt", "savefile2.txt");
        }
        if (index == "3") {
            remove("savefile3.txt");
            returnint = rename("savefile3temp.txt", "savefile3.txt");
        }
        return true;
    }
    else
        return false;
}

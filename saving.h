#include <string>
#include <fstream>
#include <vector>

struct saveState {
    int stage, coins, stamina, sanity, health, playerDmg, playerRes;
    std::map<int,int> stageEncounters = {
        {1, 1},             // In stage 1 ; Tutorial     
        {10, 5},            // In stage 10 ; duelist
        {15, 13},           // In stage 15 ; Angry man
    };
    std::vector<int> inventory;
};

int stageOfSave(std::string index) {
    std::fstream f("saves/savefile" + index + ".txt", std::ios::in);
    std::string line;
    if (f.is_open()) {
        getline(f, line);
        f.close();
        return std::stoi(line);
    }
    else
        return 0;
}

std::pair<bool, saveState> loadGame(std::string index) {
    std::fstream f("saves/savefile" + index + ".txt", std::ios::in);
    std::vector<std::string> lines;
    std::string line;
    if (f.is_open()) {
        saveState s;
        std::getline(f, line);
        while (f) {
            lines.push_back(line);
            std::getline(f, line);
        }
        f.close();
        s.stage = std::stoi(lines[0]);
        s.coins = std::stoi(lines[1]);
        s.stamina = std::stoi(lines[2]);
        s.sanity = std::stoi(lines[3]);
        s.health = std::stoi(lines[4]);
        s.playerDmg = std::stoi(lines[5]);
        s.playerRes = std::stoi(lines[6]);
        int i = 7;
        for (auto& x : s.stageEncounters) {
            x.second = std::stoi(lines[i]);
            i++;
        }
        for (int j = i; i < j+10; i++)
            if (lines[i] != "0") {
                s.inventory.push_back(std::stoi(lines[i]));
            }
        return {true, s};
    }
    else
        return {false, saveState()};
}

bool saveGame(std::string index, saveState& s, int inventorymax) {
    std::fstream f("saves/savefile" + index + "temp.txt", std::ios::out);
    std::string fileData = "";
    fileData += std::to_string(s.stage) + "\n";
    fileData += std::to_string(s.coins) + "\n";
    fileData += std::to_string(s.stamina) + "\n";
    fileData += std::to_string(s.sanity) + "\n";
    fileData += std::to_string(s.health) + "\n";
    fileData += std::to_string(s.playerDmg) + "\n";
    fileData += std::to_string(s.playerRes) + "\n";
    for (auto const& x : s.stageEncounters)
        fileData += std::to_string(x.second) + "\n";
    for (int i = 0; i < (int)s.inventory.size(); i++)
        fileData += std::to_string(s.inventory[i]) + "\n";
    for (int i = 0; i < (int)(inventorymax - s.inventory.size()); i++)
        fileData += "0\n";
    if (f.is_open()) {
        for (int i = 0; i < (int)fileData.size(); i++) {
            f.put(fileData[i]);
        }
        f.close();
        int returnint;
        if (index == "1") {
            remove("saves/savefile1.txt");
            returnint = rename("saves/savefile1temp.txt", "saves/savefile1.txt");
        }
        if (index == "2") {
            remove("saves/savefile2.txt");
            returnint = rename("saves/savefile2temp.txt", "saves/savefile2.txt");
        }
        if (index == "3") {
            remove("saves/savefile3.txt");
            returnint = rename("saves/savefile3temp.txt", "saves/savefile3.txt");
        }
        return true;
    }
    else
        return false;
}

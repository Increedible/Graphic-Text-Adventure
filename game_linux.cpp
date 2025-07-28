#include <iostream>
#include <stdio.h>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <stack>
#include <thread>
#include <unistd.h>
#include <assert.h>
#include <climits>
#include "assets.h"
#include <random>
using namespace std;

#ifdef _WIN32
    #include "input_windows.h"
#elif __linux__
    #include "input_linux.h"
#endif

my_io io;

const double FPS = 17.0; // Default: 15.0
const int X_SIZE = 101;
const int Y_SIZE = 11;

const int inventorymax = 10;
const int defCoins = 10;
const int defStamina = 10;
const int defHealth = 100;
const int defSanity = 10;
bool checkedMap = false;
int stage = 0;
int prevstage = stage;
int coins = 10;
int stamina = defStamina;
int sanity = defSanity;
int health = defHealth;
int moral = 0;
const int defPlayerDmg = 50;
const int defPlayerRes = 0;
int playerDmg = defPlayerDmg;
int playerRes = defPlayerRes;
bool respawn = false;
vector<int> inventory;
string currentFileIndex = "1";

template<class T, size_t N>
constexpr size_t lengthof(T(&)[N]) { return N; }

mt19937 rng(time(0));
// [l, r)
int my_rand(int l, int r) {
    uniform_int_distribution gen(l, r-1);
    return gen(rng);
}
// [0,r)
int my_rand(int r) {
    return my_rand(0, r);
}

void goBack(int b) {
    printf("\033[%dA", b);
}

void set_cursor(bool visible) {
    if (visible) {
        printf("\e[?25h");
    } else {
        printf("\e[?25l");
    }
}

void clear() {
    system("clear");
    printf("\e[H");
}

void getCin() {
    do {
        io.check_sync();
    } while (!io.pressed[K_ENTER]);
}

string toString(char c) {
    string s(1, c);
    return s;
}

string colored(string text, string type, string color, string color2 = "NULL") {
    vector<int> values;
    bool istext = false;
    bool isback = false;
    stack<string> colors;
    if (type == "text") {
        istext = true;
        colors.push(color);
    }
    else if (type == "back") {
        isback = true;
        colors.push(color);
    }
    else if (type == "backtext") {
        istext = true;
        isback = true;
        colors.push(color);
        if (color2 == "NULL") { istext = false; }
        else { colors.push(color2); }
    }
    else if (type == "textback") {
        istext, isback = true;
        if (color2 == "NULL") { return "ERROR : Need second color"; }
        colors.push(color2);
        colors.push(color);
    }
    else
        return "ERROR: Unvalid type";
    if (istext) {
        string pickedcolor = colors.top();
        colors.pop();
        if (pickedcolor == "black") { values.push_back(30); }
        else if (pickedcolor == "red") { values.push_back(31); }
        else if (pickedcolor == "green") { values.push_back(32); }
        else if (pickedcolor == "yellow") { values.push_back(33); }
        else if (pickedcolor == "blue") { values.push_back(34); }
        else if (pickedcolor == "magenta") { values.push_back(35); }
        else if (pickedcolor == "cyan") { values.push_back(36); }
        else if (pickedcolor == "white") { values.push_back(37); }
        else { values.push_back(30); }
    }
    if (isback) {
        string pickedcolor = colors.top();
        colors.pop();
        if (pickedcolor == "black") { values.push_back(40); }
        else if (pickedcolor == "red") { values.push_back(41); }
        else if (pickedcolor == "green") { values.push_back(42); }
        else if (pickedcolor == "yellow") { values.push_back(43); }
        else if (pickedcolor == "blue") { values.push_back(44); }
        else if (pickedcolor == "magenta") { values.push_back(45); }
        else if (pickedcolor == "cyan") { values.push_back(46); }
        else if (pickedcolor == "white") { values.push_back(47); }
        else { values.push_back(30); }
    }
    values.push_back(1);
    string output;
    for (int i = 0; i < (int)values.size(); i++) {
        output += to_string(values[i]);
        if (i < (int)values.size() - 1) { output += ';'; }
    }
    return ("\033[" + output + 'm' + text + "\033[0m");
}

#define PIXEL_BLACK "\033[40m \033[0m"
#define PIXEL_RED "\033[41m \033[0m"
#define PIXEL_GREEN "\033[42m \033[0m"
#define PIXEL_YELLOW "\033[43m \033[0m"
#define PIXEL_BLUE "\033[44m \033[0m"
#define PIXEL_MAGENTA "\033[45m \033[0m"
#define PIXEL_CYAN "\033[46m \033[0m"
#define PIXEL_WHITE "\033[47m \033[0m"

bool isAlphabet(char c) {
    string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12456789!@#$%^&*()<>,.''/\"";
    return allowed.find(c) != std::string::npos;
}

void MSDelay(int d) { usleep(d*1000); }

void typeOut(string text, int sleepms = 18, int aftersleep = 0) {
    bool skip = 0;
    for (char c : text) {
        cout << c;
        io.check();;
        if (io.pressed[K_LEFT])skip = 1;
        if (c == '\n')skip = 0;
        if (!skip && isAlphabet(c)) {
            MSDelay(sleepms);
        }
    }
    cout << endl;
    MSDelay(aftersleep * 1000);
}

string optionsNav(map<string, string> options, map<string, string> specialOptions, string option = "select", bool (*func1)(int) = nullptr, bool (*func2)(int) = nullptr) {
    int iteration = 0;
    bool rerender = true;
    //cout << "Use up and down arrow keys to navigate, right arrow key to pick." << endl;
    set_cursor(false);
    do {
        io.check_sync();
        if (rerender) {
            rerender = false;
            int i = 0;
            for (auto const& p : options) {
                if (iteration == i)
                    cout << colored(option + " >\t" + p.second, "text", "yellow") << endl;
                else if (func1 == nullptr || ((func2 != nullptr) && !func2(i)))
                    cout << colored("      \t" + p.second, "text", "white") << endl;
                else if (func1(i))
                    cout << colored("      \t" + p.second, "text", "green") << endl;
                else
                    cout << colored("      \t" + p.second, "text", "red") << endl;
                i++;
            }
            for (auto const& p : specialOptions) {
                if (iteration == i)
                    cout << colored(option + " >\t" + p.second, "text", "yellow") << endl;
                else
                    cout << colored("       \t" + p.second, "text", "blue") << endl;
                i++;
            }
            goBack((options.size() + specialOptions.size()));
        }
        if (io.pressed[K_UP])
            if (iteration > 0) {
                iteration--;
                rerender = true;
            }
        if (io.pressed[K_DOWN])
            if (iteration < (int)options.size() + (int)specialOptions.size() - 1) {
                iteration++;
                rerender = true;
            }
    } while(!io.pressed[K_RIGHT]);
    for (int i = 0; i < (int)options.size() + (int)specialOptions.size(); i++)
        cout << endl;
    set_cursor(true);
    int i = 0;
    for (auto const& p : options) {
        if (iteration == i)
            return toString(p.first[p.first.length() - 1]);
        i++;
    }
    for (auto const& p : specialOptions) {
        if (iteration == i)
            return toString(p.first[p.first.length() - 1]);
        i++;
    }
    return "OPTIONSNAV ERROR";
}

// Stages
string stages[] = {
    "Starter area",     // 0
    "Mountains",        // 1
    "Big Mountain",     // 2
    "River Stream",     // 3
    "Stone Path",       // 4
    "Treasure Chest",   // 5
    "Abandoned House",  // 6
    "Gate",             // 7
    "Main village hub", // 8
    "Shop",             // 9
    "Angry Duelist",    // 10
    "Creepy Alley",     // 11
    "Street Act",       // 12
    "West Wall",        // 13
    "Coal Mine",        // 14
    "Club",             // 15
    "Animal Fight",     // 16
};

/*  =========================
        Dialogue
========================= */

string deadend[] = {
    "Walking here you can't see anything due to the thick fog. Anxious that you might trip and fall down a cliff, you return.",
    "Walking here makes you feel uncomfortable, as it's a very steep way down. You decide it's too risky.",
    "Sometimes it's better to not be greedy and take what you have, who knows what dangers lie beyond.",
    "You were about to step forward when you heard a howl. You decide it's not a good idea to head this way.",
    "Are you sure you want to head off path? It doesn't look like a good idea.",
    "You are not so sure if it's a good idea to head off path... where should you go?",
    "You head that way and bump yourself into a wall... maybe you should have thought twice.",
    "You look that way and see a solid gate... what were you thinking heading that way?",
    "You see a dark void further in the mine and state its dangerously steep, so you decide it's better to not go that way.",
};

string dialogue[lengthof(stages)] = {
    "You find yourself seated on top of a rock, looking around you you can only see thick fog. \nYou are armed with a map and compass, and some " + colored("coins", "text", "yellow") + ".",
    "You feel a fresh but cold wind breezing over your neck, and around you you see only white. \nYou read carven on a stone that you can press the left arrow key to make me talk faster.\nThe old man left a note: 'You will encounter enemies randomly, defeat them for coins!'\nYou spot a bird in the distance.",
    "You climbed this beast of a mountain, enourmous. \nBut you also knew, that you are so high up here that one mistep could be catastrophic.",
    "You travel along a silent river stream. \nYou suspect it gets its water from smelting ice up another mountain.",
    "You encounter a stone path, that probably leads to something promising, a form of life or civilization maybe?",
    "You approach something shiny, wow! Its a treasure chest! \nYou are very lucky that you found something this helpful this early. \nYou grab what you need and are sattisfied your efforts paid off.",
    "You see a cracky abandoned house and enter it. \nYou try to scout for loot, but can only find a dusty key.",
    "You see in front of you an enourmous gate. \nIt says that inside you can find a village. But you need to pay the guard a fee first.\nHe approaches you and explains that you need " + colored("20", "text", "yellow") + " coins to enter.",
    "You find yourself in the main village hub. \nYou see a lovely tree in the middle, and there are lots of ways to go.",
    "You enter building which claims it to be a shop.\nIndeed, when you enter, you are greeted to a what seems to be a friendly man.",
    "You fended off the duelist. The surrounding villagers are very pleased about your act. \nThere will be peace here, for now...",
    "You enter a creepy alley. It's dark and looks abadoned.\nIf this would be the only part of the village you'd be allowed to see, you'd think it's empty and dead.",
    "You see two artists acting on the street. \nYou stand and watch, wondering if they will be performing here for eternity...",
    "You stand before a solid, big wall. You read, carved on the stone: \n'Here stands, the West Wall!'\nYou notice the first half of the wall is made of smooth stone, while the other half is made of cobblestone.\nYou wonder if the second half was built for precaution, like someone tried to climb over it in the past...",
    "You enter a mine, with the indistinguishable smell of coal.\nIt's less aromatic than wood smoke and has a dirtier smell, like cigar smoke compared to a pipe mixture.",
    "You enter a local pub, where old and young men sit down and have a laugh together while drinking.\nYou see a door in the corner, and a man in a black jacket entering it.",
    "",
};

/*  =========================
            Directions
    ========================= */

int directions[lengthof(stages)][4] = {
    //  Negative numbers mean dead ends
    //  {north, east, south, west }
        {1, -1, -1, -1},    // 0 ; Starter area
        {4, 3, 0, 2},       // 1 ; Mountains
        {-2, 1, -2, 5},     // 2 ; Big mountain
        {6, -2, -2, 1},     // 3 ; River stream
        {7, -5, 1, -5},     // 4 ; Stone path
        {-3, 2, -3, -3},    // 5 ; Treasure chest
        {-4, -4, 3, -4},    // 6 ; Abandoned House
        {8, -5, 4, -5},     // 7 ; Gate
        {-1, 9, 7, 10},     // 8 ; Main village hub
        {-7, -7, -7, 8},    // 9 ; Shop
        {12, 8, 11, 13},    // 10 ; Angry Duelist
        {10, -8, -1, 14},   // 11 ; Creepy Alley
        {-1, -1, 10, -1},   // 12 ; Street Act
        {-1, 10, 14, -8},   // 13 ; West wall
        {13, 11, -9, -9},   // 14 ; Coal Mine
        {11, -7, 16, -7},   // 15 ; Club
        {15, -7, -7, -7},   // 16 ; Animal Fight
};

/*  =========================
            Items
    ========================= */

struct item {
    int id;
    string name;
    string description;
    bool isdroppable;
    bool isusable;
    int healthrestore=0; // If isedible, how much health does it restore?
    vector<int> iterate;
    item(int tid, const string& tname, const string& tdescription, bool tisdroppable, bool tisusable, int thealthrestore=0) {
        id=tid;
        name=tname;
        description=tdescription;
        isdroppable=tisdroppable;
        tisusable=tisusable;
        healthrestore=thealthrestore;
        iterate = {isdroppable, isusable, healthrestore};
    }
};

item items[6] = {
    {0, "NULL", "", false, false, false},
    {1, "Dusty Key", "A key that looks like it hasn't been used in a long time.", true, false, false},
    {2, "Premium Pass", "A pass that allows you to enter the club.", true, false, false},
    {3, "Wooden Planks", "A few wooden planks that can be used for crafting.", true, true, false},
    {4, "Red Balloon", "A red balloon that can be used to cheer up someone.", true, true, false},
    {5, "Oven Cookie", "A delicious cookie that restores some health when eaten.", true, false, 25},
};

map<int, int> needItem {
    {5, 1},         // for treasure chest, need key
    {15, 2},        // to enter club, need premium pass           
};

map<int, int> getItem {
    {6, 1}      // Get key in house
};

/*  =========================
            Inventory
    ========================= */

bool addToInventory(int item) {
    if (inventory.size() >= inventorymax)
        return false;
    inventory.push_back(item);
    return true;
}

bool inventoryHas(int item) {
    for (int i:inventory) {
        if (i==item) return true;
    }
    return false;
}

int inputint = 0;

void interactInventory() {
    bool exit = false;
    while (!exit) {
        map<string, string> options;
        map<string, string> specialOptions;
        if (inventory.size() <= 0)
            typeOut("\nYour inventory is empty.");
        else {
            typeOut("\nYour items:");
            for (int i = 0; i < (int)inventory.size(); i++)
                options.insert(pair<string, string>(to_string(i + 1), to_string(i + 1) + ".\t" + items[inventory[i]].name));
        }
        specialOptions.insert(pair<string, string>("1e", "Exit"));
        string input = optionsNav(options, specialOptions, "Inv");
        if (input == "e")
            exit = true;
        else if (isdigit(input[0])) {
            inputint = input[0] - '0';
            int size = inventory.size();
            if (inputint <= size && inputint > 0) { // Buy succeeded, now buy item if money
                inputint -= 1; // Correct iterator
                typeOut("\n" + colored(items[inventory[inputint]].name, "text", "magenta"));
                map<string, string> options2;
                map<string, string> specialOptions2;
                options2.insert(pair<string, string>("1d", "Drop"));
                options2.insert(pair<string, string>("2u", "Use"));
                options2.insert(pair<string, string>("3e", "Eat"));
                options2.insert(pair<string, string>("4x", "Exit"));
                string input = optionsNav(options2, specialOptions2, "Item", [](int i) { return (items[inventory[inputint]].iterate[i] != 0); });
                if (input == "d") {
                    if (items[inventory[inputint]].isdroppable) {
                        int invInputInt = inventory[inputint];
                        inventory.erase(inventory.begin() + inputint);
                        typeOut("Succesfully dropped " + colored(items[invInputInt].name,"text","magenta") + "! [Press Enter]");
                    }
                    else
                        typeOut("This item cannot be dropped! [Press Enter]");
                    getCin();
                }
                if (input == "u") {
                    if (items[inventory[inputint]].isusable) {
                        // use the item, somehow
                        int invInputInt = inventory[inputint];
                        inventory.erase(inventory.begin() + inputint);
                        typeOut("Succesfully used " + colored(items[invInputInt].name, "text", "magenta") + "! [Press Enter]");
                    }
                    else
                        typeOut("This item cannot be used! [Press Enter]");
                    getCin();
                }
                if (input == "e") {
                    if (items[inventory[inputint]].healthrestore>0) {
                        int invInputInt = inventory[inputint];
                        inventory.erase(inventory.begin() + inputint);
                        if (health + items[invInputInt].healthrestore > defHealth) {
                            health = 100;
                            typeOut("You eat the " + colored(items[invInputInt].name, "text", "magenta") + ".");
                            typeOut("Your health was set back to " + colored(to_string(health), "text", "red") + ". [Press Enter]");
                        }
                        else {
                            health += items[invInputInt].healthrestore;
                            typeOut("You eat the " + colored(items[invInputInt].name, "text", "magenta") + ".");
                            typeOut("Your health was increased and now is " + colored(to_string(health), "text", "red") + ". [Press Enter]");
                        }
                    }
                    else
                        typeOut("This item cannot be eaten! [Press Enter]");
                    getCin();
                }
                if (input == "x") {}
            }
            else
                typeOut("The item number " + to_string(inputint) + " is not available!\n");
        }
        else
            typeOut("I am not sure what you meant with '" + input + "', but its not a valid option.\n");
    }
}

/*  =========================
            Coins
    ========================= */

map<int,int> coinrewards {
    {5, 10},         // Get coins in treasure chest
    {10, 20}         // You get reward for fending off duelist
};

map<int,int> needcoins {
    {8, 20}         // To get in village hub, need 20 coins to pass through gate.
};

/*  =========================
            Shop
    ========================= */

struct shop {
    vector<int> shopshellprices; // for every item put sell price in
    vector<int> shopbuyitemoptions;  // {length, rest...}
    vector<int> shopitemprices; // prices for items if you buy them
    vector<int> shopitemstock; // stock of those items
};

shop shops[2] = {
    {
        {0, 0, 0, 0, 0}, // sell prices for items
        {0}, // buy items options
        {0}, // buy prices for items
        {0} // stock of items in shop
    },
    {
        {1, 20, 5, 5, 2},
        {4, 2, 3, 4, 5},
        {0, 20, 5, 5, 2},
        {0, 1, 5, 2, 5}
    }
};

map<int,int> shopList {
    {9, 1} // on stage 9 shop #1
};

void openShop() {
    bool giveOptions = false;
    bool exit = false;
    while (!exit) {
        typeOut("\nCoins: " + colored(to_string(coins), "text", "yellow"));
        map<string, string> options;
        map<string, string> specialOptions;
        cout << "\tItem\t\tPrice\tStock\n";
        for (int i = 1; i < shops[shopList[stage]].shopbuyitemoptions[0]+1; i++)
            options.insert(pair<string, string>(to_string(i+1), items[shops[shopList[stage]].shopbuyitemoptions[i]].name + "\t" + to_string(shops[shopList[stage]].shopitemprices[i]) + "\t" + to_string(shops[shopList[stage]].shopitemstock[i])));
        specialOptions.insert(pair<string, string>("1s", "Sell item(s)"));
        specialOptions.insert(pair<string, string>("2i", "View inventory"));
        specialOptions.insert(pair<string, string>("3e", "Exit"));
        string input = optionsNav(options, specialOptions, "Shop", [](int i) { return (inventory.size() < inventorymax && shops[shopList[stage]].shopitemprices[i + 1] <= coins && shops[shopList[stage]].shopitemstock[i + 1] > 0); });
        if (input == "e")
            exit = true;
        else if (input == "s") {
            bool exit = false;
            while (!exit) {
                map<string, string> options;
                map<string, string> specialOptions;
                if (inventory.size() <= 0)
                    typeOut("\nYour inventory is empty.");
                else {
                    typeOut("\nYour items:");
                    for (int i = 0; i < (int)inventory.size(); i++)
                        options.insert(pair<string, string>(to_string(i + 1), to_string(i + 1) + ".\t" + items[inventory[i]].name));
                }
                specialOptions.insert(pair<string, string>("1e", "Exit"));
                string input = optionsNav(options, specialOptions, "Sell");
                if (input == "e")
                    exit = true;
                else if (isdigit(input[0])) {
                    int inputint = input[0] - '0';
                    int size = inventory.size();
                    if (inputint <= size && inputint > 0) { // Buy succeeded, now sell item
                        inputint -= 1; // Correct iterator
                        int invInputInt = inventory[inputint];
                        inventory.erase(inventory.begin() + inputint);
                        coins += shops[shopList[stage]].shopshellprices[invInputInt-1];
                        typeOut("You went ahead gained " + colored(to_string(shops[shopList[stage]].shopitemprices[invInputInt-1]), "text", "yellow") + ((shops[shopList[stage]].shopitemprices[invInputInt-1] == 1) ? "coin" : "coin(s)") + "for selling your " + colored(items[invInputInt].name, "text", "magenta") + ".\n");
                        
                    }
                    else
                        typeOut("The item number " + to_string(inputint) + " is not available!\n");
                }
                else
                    typeOut("I am not sure what you meant with '" + input + "', but its not a valid option.\n");
            }
        }
        else if (input == "i") {
            interactInventory();
        }
        else if (isdigit(input[0])) { 
            int inputint = input[0] - '0';
            int size = shops[shopList[stage]].shopbuyitemoptions.size();
            if (inputint <= size && inputint > 0) { // Buy succeeded, now buy item if money
                inputint -= 1; // Correct iterator
                if (shops[shopList[stage]].shopitemstock[inputint] > 0) {
                    if (shops[shopList[stage]].shopitemprices[inputint] <= coins) {
                        if (addToInventory(shops[shopList[stage]].shopbuyitemoptions[inputint]) == true) {
                            coins -= shops[shopList[stage]].shopitemprices[inputint];
                            shops[shopList[stage]].shopitemstock[inputint] -= 1;
                            if (shops[shopList[stage]].shopitemstock[inputint] == 0) { giveOptions = true; }
                            typeOut("\nYou went ahead and bought a " + colored(items[shops[shopList[stage]].shopbuyitemoptions[inputint]].name, "text", "magenta") + ", and paid the shop owner " + colored(to_string(shops[shopList[stage]].shopitemprices[inputint]), "text", "yellow") + " coins.\n");
                        }
                        else
                            typeOut("You wanted to buy a " + colored(items[shops[shopList[stage]].shopbuyitemoptions[inputint]].name, "text", "magenta") + ", but your inventory was full!\n");
                    }
                    else
                        typeOut("You wanted to buy a " + colored(items[shops[shopList[stage]].shopbuyitemoptions[inputint]].name, "text", "magenta") + ", but you didn't have enough coins!\n");
                }
                else
                    typeOut("You wanted to buy a " + colored(items[shops[shopList[stage]].shopbuyitemoptions[inputint]].name, "text", "magenta") + ", but it was out of stock!\n");
            }
            else
                typeOut("The item number " + to_string(inputint) + " is not available!\n");
        }
        else
            typeOut("I am not sure what you meant with '" + input + "', but its not a valid option.\n");
    }
    typeOut("You exit the shop menu.\n");
}

string minigameToString(int minigamenmr) {
    ostringstream os;
    for (int i = 0; i < (int)minigameVis[minigamenmr].size(); i++) {
        for (int j = 0; j < (int)minigameVis[minigamenmr][i].length(); j++) {
            if (minigameVis[minigamenmr][i][j] == '0')
                os << PIXEL_GREEN;
            else if (minigameVis[minigamenmr][i][j] == '1')
                os << PIXEL_WHITE;
            else if (minigameVis[minigamenmr][i][j] == '2')
                os << PIXEL_YELLOW;
            else if (minigameVis[minigamenmr][i][j] == '3')
                os << PIXEL_BLUE;
            else if (minigameVis[minigamenmr][i][j] == '4')
                os << PIXEL_RED;
            else if (minigameVis[minigamenmr][i][j] == '5')
                os << PIXEL_MAGENTA;
            else if (minigameVis[minigamenmr][i][j] == '6')
                os << PIXEL_BLACK;
            else if (minigameVis[minigamenmr][i][j] == '7')
                os << PIXEL_CYAN;
        }
        os << endl;
    }
    return os.str();
}

map<int, int> minigames {
    {14, 1} // on stage 14 minigame #1 : miner
};

string minigametext[] = {
    "NULL",             // minigame #0 doesn't exist
    "Mine some gold",   // minigame #1
};

void doMinigame() {
    if (minigames[stage] == 1) {
        typeOut("A local miner offers you to strike a boulder to maybe find gold!");
        cout << minigameToString(0) << endl;
        typeOut("Pick one boulder to mine.");
        map<string, string> options;
        map<string, string> specialOptions;
        specialOptions.insert(pair<string, string>("1", "First Boulder"));
        specialOptions.insert(pair<string, string>("2", "Second Boulder"));
        specialOptions.insert(pair<string, string>("3", "Third Boulder"));
        string input = optionsNav(options, specialOptions, "Mine");
        int inputint = input[0] - '0';
        if (my_rand(3) == 0) {
            cout << minigameToString(inputint+3);
            coins += 20;
            if (inputint == 1)
                typeOut("You strike the first boulder and find some weird, valuable ore!");
            if (inputint == 2)
                typeOut("You strike the second boulder and find some weird, valuable ore!");
            if (inputint == 3)
                typeOut("You strike the third boulder and find some weird, valuable ore!");
            typeOut("You sell it and gain " + colored("20", "text", "yellow") + " coins. [Press Enter]");
        }
        else {
            cout << minigameToString(inputint);
            if (inputint == 1)
                typeOut("You strike the first boulder and find nothing, unfortunately. [Press Enter]");
            if (inputint == 2)
                typeOut("You strike the second boulder and find a worm, but nothing of value. [Press Enter]");
            if (inputint == 3)
                typeOut("You strike the third boulder and get yourself nothing but painful arms. [Press Enter]");
        }
        getCin();
    }
}

/*  =========================
               Map
    ========================= */

string compass[] = {
    " Legend:",
    " Item\tType",
    "",
    "",
    "",
    "",
    "",
    "",                                       // Compass
    "",
    "",
    "",
    "      N",
    "      ^",
    "  W < X > E",
    "      v",
    "      S"
};

string mapvis[lengthof(stages)][16] = {
    {"2121212121212121212121212121212121",
     "2000000000000002220000000000000002",
     "1000000000000022220000000000000001",
     "2000000000000002222000000000000002",
     "1000000000000022220000000000000001",    
     "2000000000000111111110000000000002",
     "1000000000011111111110000000000001",
     "2000000000111111Y11111000000000002",        // 0 ; Starter area
     "1000000000111111111111100000000001",
     "2000000000011111111110000000000002",
     "1000000000011111111000000000000001",
     "2000000000001111110000000000000002",
     "1000000000000000000000000000000001",
     "2000000000000000000000000000000002",
     "1000000000000000000000000000000001",
     "2121212121212121212121212121212122"
    },
    {"2121212121212121212121212121212121",
     "2111100000000002220000000000000002",
     "1111111000000022220000000000000001",
     "2111111110000002222000001110000002",
     "1111111111100022220000001111100001",
     "2111111111110111111110011111110002",
     "1111111111011111111110000000000001",
     "2222222222111111Y11111222222222222",        // 1 ; Mountains
     "1222222222111111111111122222222221",
     "2000000000011111111110000000000002",
     "1000110000011111111000001111000001",
     "2011111100001111110000111111110002",
     "1011111110000022200000011111110001",
     "2011111100000022200000001111100002",
     "1001111000000022200000000000000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000000000000000000000000000000002",
     "1000000000111111111111100000000001",
     "2000000011111111111111111100000002",
     "1000000111111111111111111111000001",
     "2000001111111111111111111111000002",
     "1222222221111111111111111111110001",
     "2222221122211111Y11111222222222222",        // 2 ; Big Mountain
     "1001111111111111111111122222222221",
     "2000001111111111111111111111100002",
     "1000001111111111111111111111110001",
     "2000000111111111111111111111100002",
     "1000000011111111111111111111000001",
     "2000000000011111111111111100000002",
     "1000000000000000000000000000000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000000000000000022223333330000002",
     "1000000000000000022220333330000001",
     "2000000000000000222220333330000002",
     "1000000000000000222220033333000001",
     "2000000000000002222220003333000002",
     "1000000000000022222200003333000001",
     "2222222222222222Y22200033333000002",        // 3 ; River stream
     "1222222222222222222000033333000001",
     "2222222222222200000000333330000002",
     "1000000000111100000000333330000001",
     "2000000011111111000000033330000002",
     "1000000011111110000000033333000001",
     "2000000001111111000000003333000002",
     "1000000000000000000000003333300001",
     "2121212121212121212121212121212122"
    },
    {"2121212121212121212121212121212121",
     "2000000000000001111100000000000002",
     "1000000000000000111110000000000001",
     "2000000000000001111100000000000002",
     "1000000000000011111000000000000001",
     "2000000000000111110000000000000002",
     "1000000000000011111000000000000001",
     "2000000000000001Y11100000000000002",        // 4 ; Stone Path
     "1000000000000001111100000000000001",
     "2000000000000000111110000000000002",
     "1000000000000000111110000000000001",
     "2000000000000001111100000000000002",
     "1000000000000001111100000000000001",
     "2000000000000011111000000000000002",
     "1000000000000011111000000000000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2111111111000000000000000000000002",
     "1111111111100000000000000000000001",
     "2000111111111000000000000000000002",
     "1100001111111110000000000000000001",
     "2110000001111111110000000000000002",
     "1111002222222222222222222222222221",
     "2111222T22222222Y22222222222222222",        // 5 ; Treasure
     "1111000222222222222222222222222221",
     "2110000000111111110000000000000002",
     "1100000011111111110000000000000001",
     "2000000011111111000000000000000002",
     "1000000111111100000000000000000001",
     "2000011111110000000000000000000002",
     "1011111111000000000000000000000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000000000000000000000000000000002",
     "1000000000000000000000000000000001",
     "2000000011111111111111111000000002",
     "1000000010000000000000001000000001",
     "2000000010000000000000001000000002",
     "1000000010000000000000001000000001",
     "2000000010000000Y00000001000000002",        // 6 ; Abandoned house
     "1000000010000000000000001000000001",
     "2000000011111112221111111000000002",
     "1000000000000001222000000000000001",
     "2000000000000000122000000000000002",
     "1000000000000000222000000000000001",
     "2000000000000000222000000000000002",
     "1000000000000000222000000000000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000000000000002220000000000000002",
     "1000000000000002220000000000000001",
     "2111111111111111111111111111111112",
     "1111111111111111111111111111111111",
     "20000000000000G2220000000000000002",
     "1000000000000022220000000000000001",
     "2000000000000022Y20000000000000002",        // 7 ; Gate
     "1000000000000022220000000000000001",
     "2000000000000022220000000000000002",
     "1000000000000002222000000000000001",
     "2000000000000002222000000000000002",
     "1000000000000002222000000000000001",
     "2000000000000002222000000000000002",
     "1000000000000022220000000000000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000333300000002220000333000000002",
     "1003333333000002220000033300000001",
     "2003333333000002220000033300000002",
     "1000333333000022222000033300000001",
     "2000003330000200000200033300000002",
     "1000000000002001110020003330000001",
     "2222222222220011Y11002222222222222",        // 8 ; Main village hub
     "1000000000002001110020003330000001",
     "2000000000000200000200033300000002",
     "1000000000000022222000033300000001",
     "2000000000000002220000033300000002",
     "1000000000000002220000033300000001",
     "2000000000000002220000003330000002",
     "1000000000000002220000003330000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000000000000000000000000000000002",
     "1000000000001111111111111111100001",
     "2000000000001222221122222222100002",
     "1000000000001222221122222222100001",
     "2000000000001222221122222222100002",
     "1222222222221222221122222222100001",
     "2222222222222222Y2112S222222100002",        // 9 ; Shop
     "1222222222001222221122222222100001",
     "2000000000001222221122222222100002",
     "1000000000001222221122222222100001",
     "2000000000001222221122222222100002",
     "1000000000001222221122222222100001",
     "2000000000001111111111111111100002",
     "1000000000000000000000000000000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000002C20000002220000222222033332",
     "100002C20000000222000002CC22333301",
     "20002C2000000001110000002CC2333302",
     "1002C20000011112221111000222333301",
     "2022200000100002220000100222333302",
     "1000000001000002220000010000333301",
     "2222222221222222Y22222212222222222",        // 10 ; Duelist
     "1000000001000002220000010000333301",
     "2000222200100002220000100000333302",
     "100022C200011112221111002222333301",
     "20002C220000000111000002CC22333302",
     "10002C2200000002220000222C22333301",
     "200022C200000002220000022223333002",
     "1000222200000002220000000033330001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000000000000002220000111333311112",
     "1111000000000002220001111333311111",
     "2111111000000002220001111333331112",
     "1111111111100002220011111133331111",
     "2111111111111102220111111133331112",
     "1111111111111112221111111333311111",
     "2222222222222222Y21111113333311112",        // 11 ; Creepy Alley
     "1111111111111112221111113333111111",
     "2111111111111112221111113333111112",
     "1111111111111112221111113333111111",
     "2111111111111112221111111333311112",
     "1111111111111112221111111133331111",
     "2111111111111112221111111133331112",
     "1111111111111112221111111133331111",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000000000000002220000000003333002",
     "1002222222220002220000000003333001",
     "20022P2222220000222000000000333302",
     "10022222P2220000222000000000333301",
     "2002222222220000222000000000333302",
     "1000000022000002220000002203333001",
     "2000222200222222Y22222220022233222",        // 12 ; Street Act
     "1222000000000002220000000003322001",
     "2000000000000002220000000003333002",
     "1000000000000000222000000003333001",
     "2000000000000000222000000000333302",
     "1000000000000000222000000000333301",
     "2000000000000002220000000000033332",
     "1000000000000002220000000000033331",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "2000001111100000222000000000000002",
     "1000001111100000222000000000000001",
     "2000001111100002220000000000000002",
     "1000001111100002220000000000000001",
     "2000001111100022220000000000000002",
     "1000001111100222222000000222000001",
     "2000001111100222Y22222222000222002",        // 13 ; West Wall
     "1000001111100222222000000000000221",
     "2000001111100022220000000000000002",
     "1000001111100002220000000000000001",
     "2000001111100002220000000000000002",
     "1000001111100000222000000000000001",
     "2000001111100000222000000000000002",
     "1000001111100000222000000000000001",
     "2121212121212121212121212121212121"
    },
    {"2121212121212121212121212121212121",
     "20000000000000222222W2222000000002",
     "1000000000000022222004122000000001",
     "20000000000000222W14141W2000000002",
     "1000000000000022204141422000000001",
     "2111111110000222222000022222222222",
     "1111111111112222222222222222222221",
     "2111122222222222Y22222111111111112",        // 14 ; Coal Mine
     "1111222211111222222221111144111111",
     "2112221411111112221111111114111412",
     "1422W41111111122211111111114111111",
     "2122111411111222411111111111111112",
     "1111141111112W21144111114411114111",
     "2111111111114111144111111441111112",
     "1111111111111111111111111111111111",
     "2121212121212121212121212121212121"
    },
};

string maplegend[lengthof(stages)][6] = {
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou",string(PIXEL_BLUE) + "\tWater"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou","T\tTreasure"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou","G\tGuard"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou","S\tShopkeeper"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou","C\tCrowd"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou","P\tPerformer"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou"},
    {string(PIXEL_GREEN) + "\tGrass",string(PIXEL_WHITE) + "\tRock",string(PIXEL_YELLOW) + "\tPath",string(PIXEL_CYAN) + "\tYou",string(PIXEL_RED)+ "\tMineral Ore","W\tWorkers"},
};

string mapconclusion[lengthof(stages)] = {
    "It looks like North is the only reasonable option.",
    "You can't see any dead ends.",
    "Going either North or South looks sketchy.",
    "You can't decide which is worse, east or south.",
    "North seems to lead to something promising, south leads you back.",
    "You should head back.",
    "You conclude you should go back.",
    "You need to head through, so you need " + colored("20", "text", "yellow") + " coins.",
    "The nice center of the village, the main hub.\nYou see to the north a big road.",
    "If you want to leave, you should head out again, right?",
    "There are plenty of ways to go.",
    "You can go north, south, or west.",
    "You should conclude: No dead ends!",
    "You can go every way except west, obviously.",
    "A local worker warns you about going deeper into the mine.",
};

string mapToString() {
    ostringstream os;
    for (int i = 0; i < (int)lengthof(maplegend[stage]); i++) {
        compass[i + 2] = " " + maplegend[stage][i];
    }
    for (int i = 0; i < (int)lengthof(mapvis[stage]); i++) {
        for (int j = 0; j < (int)mapvis[stage][i].length(); j++) {
            if (mapvis[stage][i][j] == '0')
                os << PIXEL_GREEN;
            else if (mapvis[stage][i][j] == '1')
                os << PIXEL_WHITE;
            else if (mapvis[stage][i][j] == '2')
                os << PIXEL_YELLOW;
            else if (mapvis[stage][i][j] == 'Y')
                os << PIXEL_CYAN;
            else if (mapvis[stage][i][j] == '3')
                os << PIXEL_BLUE;
            else if (mapvis[stage][i][j] == '4')
                os << PIXEL_RED;
            else 
                os << colored(toString(mapvis[stage][i][j]), "backtext", "yellow");
        }
        if (compass[i] != "")
            os << compass[i];
        os << "\n";
    }
    os << "\n" << mapconclusion[stage] << "\n";
    return os.str();
}

/*  =========================
              Rest
    ========================= */

void rest() {
    typeOut("You decided to lie down and look up to the clear sky and rest for a bit.\nYou rest your eyes.\nResting...");
    set_cursor(false);
    double progress = 0.0;
    while (progress <= 1.0) {
        double barWidth = 70;
        double pos = barWidth * progress;
        for (int i = 0; i < (int)barWidth; i++) {
            if (i <= pos) std::cout << PIXEL_RED;
            else std::cout << PIXEL_WHITE;
        }
        cout << " " << int(progress * 100.0) << " %\r";
        cout.flush();

        progress += 0.01;
        MSDelay(100);
    }
    for (int i = 0; i < 70; i++) {
        cout << PIXEL_RED;
    }
    cout << " 100%\n";
    set_cursor(true);
    stamina = defStamina;
    if (sanity > 7) {
        sanity = 10;
        typeOut("You are done resting! You feel a lot better.\nYour stamina reset back to " + colored(to_string(defStamina), "text", "blue") + ".\nYour sanity was set to " + colored("10", "text", "red") + ".");
    }
    else {
        sanity += 3;
        typeOut("You are done resting! You feel a lot better.\nYour stamina reset back to " + colored(to_string(defStamina), "text", "blue") + ".\nYour sanity increased by " + colored("3", "text", "red") + ".");
    }
}

/*  =========================
         Battle System
    ========================= */

string battleVis[3] = {
    "666666666222222222000000000444444444544444444000000000222222222666666666",
    "666666666222222222000000000444444444544444444000000000222222222666666666",
    "666666666222222222000000000444444444544444444000000000222222222666666666"
};

struct Enemy {
    int hp;
    int attackdurance;
    int attackdmg;
    int type; // 0 = Spikes, 1 = Melee, 2 = Ranged
    int difficulty; // 1-10
    int dmgrange; // 0 = no range, 10 = max range
    int resistance; // 0-100%
    int coinrewardmin;
    int coinrewardmax;
};

Enemy opponents[] = {
//  type 0 = Spikes
//  hp, attackdurance, attackdmg, type, difficulty, dmgrange, resistance, coinrewardmin, coinrewardmax
    {50,  200/3, 1, 0, 1, 0, 0, 0, 0},         // enemy #0
    {100, 200/3, 10, 0, 2, 10, 0, 1, 3},      // enemy #1
    {100, 400/3, 20, 0, 2, 20, 0, 2, 4},      // enemy #2
    {150, 300/3, 20, 0, 3, 30, 20, 3, 5},     // enemy #3
    {200, 300/3, 20, 0, 4, 20, 30, 3, 5},     // enemy #4
    {100, 300/3, 5, 1, 1, 10, 0, 1, 5},       // enemy #5
    {150, 300/3, 30, 1, 2, 20, 0, 3, 5},      // enemy #6
    {200, 300/3, 20, 1, 3, 50, 20, 4, 5},     // enemy #7
    {200, 300/3, 30, 1, 4, 20, 10, 3, 8},     // enemy #8
    {100, 300/3, 20, 2, 1, 20, 0, 1, 5},      // enemy #9
    {150, 300/3, 20, 2, 2, 20, 30, 3, 6},     // enemy #10
    {150, 300/3, 20, 2, 3, 20, 20, 2, 5},     // enemy #11
    {200, 300/3, 20, 2, 4, 20, 40, 4, 7},     // enemy #12
    {180, 100/3, 40, 1, 10, 40, 30, 10, 10},   // enemy #13
};

string opponentVis[lengthof(opponents)][23] = {
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333111111333333333333322222223333333333333311113333333333333",
     "33333333111111111133333333333222222222333333333333111111333333111333",
     "33331111333311111333333333333222222222333333333333311113333331111133",
     "33311111113333333333333333333332222233333333333333333333333333311133",
     "33333111133333333333333333332222222222233333333333333333333333333333",
     "33333333333333333333333322222222222222222222333333333333333333333333",
     "00000000000000000000001222222222222222222222211000000000000000000000",
     "00000000000000000000111221122222222222221112211100000000000000000000",        
     "00000000000000000001112211122222222222221111221110000000000000000000",
     "00000000000000000011122211122221111122221111222211100000000000000000",
     "00000000000000001111122111122211111112221111266211111000000000000000",
     "00000000000000111111222211222111111111222111116111111110000000000000",
     "00000000000011111111122111222111111111222111116111111111000000000000",    
     "00000000000111111111111111222111111111222111116111111111110000000000",    // 0 ; Tutorial
     "00000000001111111111111112222211111112222211111611111111110000000000",
     "00000000111111111111111122222111111111222221111611111111111100000000",
     "00000011111111111111111111111111111111111111111611111111111111000000",
     "00000111111111111111111111111111111111111111111111111111111111100000",
     "00011111111111111111111111111111111111111111111111111111111111111000",
     "00111111111111111111111111111111111111111111111111111111111111111100",
     "01111111111111111111111111111111111111111111111111111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333111333333333333333333333111113333333333333333333333333333333333",
     "33311111113333333333333333311111111133333333333113333333331111133333",
     "33333311113333333333333333333111111333333333311111333333333111113333",
     "33333333333333333333333333333333333333333333333113333333331111133333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "00000000000000000000001122211111111111112221111000000000000000000000",
     "00000000000000000000111112222111111111222211111100000000000000000000",
     "00000000000000000001111111122222222222221111111110000000000000000000",
     "00000000000000000011111111112262222262211111111111100000000000000000",
     "00000000000000001111111122222246222642222111111111111000000000000000",
     "00000000000000111111111222122222222222222221111111111110000000000000",
     "00000000000011111111112221112224444442211222111111111111000000000000",
     "00000000000111111111112221111222222222211222111111111111110000000000",    // 1 ; Hound #1
     "00000000001111111111122222111111111111112222211111111111110000000000",
     "00000000111111111111111111111111111111111111111111111111111100000000",
     "00000011111111111111111111111111111111111111111111111111111111000000",
     "00000111111111111111111111111111111111111111111111111111111111100000",
     "00011111111111111111111111111111111111111111111111111111111111111000",
     "00111111111111111111111111111111111111111111111111111111111111111100",
     "01111111111111111111111111111111111111111111111111111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"33333333333333333333113333333333333333333333333333333333333333333333",
     "33333333333333333331111113336666666666633333333333333333333333333333",
     "33333333311113333333111333366666666666663333333333331111133333333333",
     "33333333111111333333333333366666222666663331333333311111133333333333",
     "33333111331111333333333333336662222266633333333331111111111113333333",
     "33331111333333333333333366662222222222666633333333331111133333333333",
     "33333333333333333333333622222222222222222266333333333333333333333333",
     "00000000000000000000006222222222222222222222611000000000000000000000",
     "00000000000000000000166226622222222222226662611100000000000000000000",
     "00000000000000000001662266622222222222226666261110000000000000000000",
     "00000000000000000011622266622226666622126666226111100000000000000000",
     "00000000000000001116622666622266666662216622266611111000000000000000",
     "00000000000000111166214141111111111111111222116661111110000000000000",
     "00000000000011111666622466222666666666216666666666111111000000000000",
     "00000000000111111666666666222666666666122666666666111111110000000000",    // 2 ; Caped Figure
     "00000000001111116666666462222266666662222266666666611111110000000000",
     "00000000111111116616616622222666166166222226616661611111111100000000",
     "00000011111111111111111411111111111111111111111111111111111111000000",
     "00000111111111111111111111111111111111111111111111111111111111100000",
     "00011111111111111111114441111111111111111111111111111111111111111000",
     "00111111111111111111144444111111111111111111111111111111111111111100",
     "01111111111111111111114441111111111111111111111111111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333111111113333333333333333333111333333333333333",
     "33333333333333333333333331111111133333333333333311111133333333333333",
     "33333331111133333333333333311111113333333333333331113333333333333333",
     "33333111111113333333333333333333333333333333333333333333333333333333",
     "33333331111113333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "00000000000000000000001122211111111111112221111000000000000000000000",
     "00000000000000000000111122222111111111222221111100000000000000000000",
     "00000000000000000001111222122222222222221222111110000000000000000000",
     "00000000000000000011112221112262222262211122211111100000000000000000",
     "00000000000000001111112222222246222642222112211111111000000000000000",
     "00000000000000111111111222122222222222222221111111111110000000000000",
     "00000000000011111111112221112224444442211222111111111111000000000000",
     "00000000000111111111112221111222222222211222111111111111110000000000",    // 3 ; Hound #2
     "00000000001111111111122222111266666666212222211111111111110000000000",
     "00000000111111111111111111111122224222111111111111111111111100000000",
     "00000011111111111111111111111111111111111111111111111111111111000000",
     "00000111111111111111111111111111114111111111111111111111111111100000",
     "00011111111111111111111111111111111111111111111111111111111111111000",
     "00111111111111111111111111111111144111111111111111111111111111111100",
     "01111111111111111111111111111111444411111111111111111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333311113333333111111333326222623333333333333333311111333333333",
     "33333333331111333333311111133224626422333333111113333331111133333333",
     "33333333333111133333111113333222222222333333331111133311111333333333",
     "33333333331113333333333333333322666223333333311113333333333333333333",
     "33333333333333333333333332222222222222222222333333333333333333333333",
     "33333333333333333333333222222262262262222222233333333333333333333333",
     "00000000000000000000002222222226626622666622221000000000000000000000",
     "00000000000000000000112222122262262262666611221100000000000000000000",
     "00000000000000000001112221122226626622222222221110000000000000000000",
     "00000000000000000011122221122221111122221122211111100000000000000000",
     "00000000000000001111122211122211111112221111111111111000000000000000",
     "00000000000000111111666611222111111111222111111111111110000000000000",
     "00000000000011111111666611222111111111222111111111111111000000000000",
     "00000000000111111111111111222111111111222111111111111111110000000000",    // 4 ; Duelist
     "00000000001111111111111112222211111112222211111111111111110000000000",
     "00000000111111111111111122222111111111222221111111111111111100000000",
     "00000011111111111111111111111111111111111111111111111111111111000000",
     "00000111111111111111111111111111111111111111111111111111111111100000",
     "00011111111111111111111111111111111111111111111111111111111111111000",
     "00111111111111111111111111111111111111111111111111111111111111111100",
     "01111111111111111111111111111111111111111111111111111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"77777777777777777777777777777444444777777777777777777777777777777777",
     "77777711111111117777777774444441141177777777777777771111777777777777",
     "77777777711111111177777444444711111117777777777777771111177777777777",
     "77771111777771111111744471111711111117711177777777777711111777777777",
     "77777111117777777777477711111111111115511117777777777777777777777777",
     "77771111777777777777777111171111111155171111177777777777777777777777",
     "77777777777777777777777111711111116661111111177777777777777777777777",
     "00000000000000000000002211111111166611112222222000000000000000000000",
     "00000000000000000000222222211155551111112222222200000000000000000000",        
     "00000000000000000002222222211515515111112222222220000000000000000000",
     "00000000000000000022222222215155252211112222222222200000000000000000",
     "00000000000000002222222222211552222221112222222222222000000000000000",
     "00000000000000222222222222115522222222111222222222222220000000000000",
     "00000000000022222222222222155222222222111222222222222222000000000000",    
     "00000000000222222222222222551222222222111222222222222222220000000000",    // 5 ; Knight
     "00000000002222222222222225511122222221111122222222222222220000000000",
     "00000000222222222222222211111222222222111112222222222222222200000000",
     "00000022222222222222222222222222222222222222222222222222222222000000",
     "00000222222222222222222222222222222222222222222222222222222222200000",
     "00022222222222222222222222222222222222222222222222222222222222222000",
     "00222222222222222222222222222222222222222222222222222222222222222200",
     "02222222222222222222222222222222222222222222222222222222222222222220",
     "22222222222222222222222222222222222222222222222222222222222222222222"
    },
    {"77777777777777777777777777777444444777777557777777777777777777777777",
     "77777711111111117777777774444441141177775557777777771111777777777777",
     "77777777711111111177777444444711111117755577777777771111177777777777",
     "77771111777771111111744471111714111417555177777777777711111777777777",
     "77777111117777777777477711111111111115551117777777777777777777777777",
     "77771111777777777777777111141111111155511111177777777777777777777777",
     "77777777777777777777777111411111115155111111177777777777777777777777",
     "00000000000000000000002111411111151551514411122000000000000000000000",
     "00000000000000000000222111411111666555114411122200000000000000000000",        
     "00000000000000000002222211111116666111111111222220000000000000000000",
     "00000000000000000022222441111115561111111114222222200000000000000000",
     "00000000000000002222222444411155111111114444222222222000000000000000",
     "00000000000000222222222444111111111111114444222222222220000000000000",
     "00000000000022222222222444111144444411114444222222222222000000000000",    
     "00000000000222222222224444111444444441114444422222222222220000000000",    // 6 ; Stronger Knight
     "00000000002222222222224444111444444444111444422222222222220000000000",
     "00000000222222222222244444111444444444111444442222222222222200000000",
     "00000022222222222222244444111444444444111444442222222222222222000000",
     "00000222222222222222444441111144444441111144444222222222222222200000",
     "00022222222222222222444411111444444444111114444222222222222222222000",
     "00222222222222222222222222222222222222222222222222222222222222222200",
     "02222222222222222222222222222222222222222222222222222222222222222220",
     "22222222222222222222222222222222222222222222222222222222222222222222"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333366666663333333333333333333333333333333",
     "33333333333333333333333333333666666666333333333333333333333333333333",
     "33333333333333333333333333333666666666333333333333333333333333333333",
     "33333333333333333333333333333336666633333333333333333333333333333333",
     "33333333333333333333333333336666666666633333333333333333333333333333",
     "33333333333333333333333366666666666666666666333333333333333333333333",
     "66660000000000006600001666666666666666666666611006600000000000000000",
     "00006660006666000660111661166666666666661116611166000000000000006666",        
     "00000006660000660661116611166666666666661111661166000000000006666666",
     "00000000000000000661166611166661111166661111666116600000000006600000",
     "66666000066600006611166111166611111116661111166116611000000000000000",
     "00000666600000116611666611666111111111666111666611111110000660000666",
     "00000000000011111661166111666111111111666111166111111111000666666666",    
     "00000000000111111111111111666111111111666111111111661111110000666600",    // 7 ; Shadowy figure
     "00000666601111111111611116666611111116666611111111166611166000000000",
     "66660000166661111111661166666111111111666661111111116661116600000000",
     "00000011111116611116666111111111111111111111111111166666611666006666",
     "00000666666111111111666611111111111111111111111111666666111116666000",
     "00666661166661111111666661111111111111111111111116666666611111111000",
     "06666611111111111111166666666111111111111111111116666666666111111100",
     "66611111111111111116666666666111111111111111111166666666666111111110",
     "66111111111111111666666666666666111111111111666666666666666661111111"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333111111113333333333333333333111333333333333333",
     "33333333333333333333333331111111133333333333333311111133333333333333",
     "33333331111133333333333333311111113333333333333331113333333333333333",
     "33333111111113333333333333333333333333333333333333333333333333333333",
     "33333331111113333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "00000000000000000000001122211111111111112221111000000000000000000000",
     "00000000000000000000111122222111111111222221111100000000000000000000",
     "00000000000000000001111222122222222222221222111110000000000000000000",
     "00000000000000000011112221112262222262211122211111100000000000000000",
     "00000000000000001111122222222246222642222112221111111000000000000000",
     "00000000000000111111212222122222222222222221212111111110000000000000",
     "00000000000011111111122221112222444422211222121111111111000000000000",
     "00000000000111111111112221111222222222211222111111111111110000000000",    // 8 ; Hound #3
     "00000000001111111111122222111261666616212222211111111111110000000000",
     "00000000111111111111121212111126666662112121211111111111111100000000",
     "00000011111111111111212121111112244221111212121111111111111111000000",
     "00000111111111111111111111111111141111111111111111111111111111100000",
     "00011111111111111111111111111111114111111111111111111111111111111000",
     "00111111111111111111111111111111141111111111111111111111111111111100",
     "01111111111111111111111111111111444411111111111111111111111111111110",
     "11111111111111111111111111111111144111111111111111111111111111111111"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "00000000000000000000001111111000111111111111111000000000000000000000",
     "00000000000000000000111111111000011111111111111100000000000000000000",        
     "00000000000000000001111111111110000111111111111110000000000000000000",
     "00000000000000000011111111111111000011111111111111100000000000000000",
     "00000000000000001111111111111111110011111111111111111000000000000000",
     "00000000000000111111111111111111106060111111111111111110000000000000",
     "00000000000011111111111111111111100000111111111111111111000000000000",    
     "00000000000111111111111111111111106460111111111111111111110000000000",    // 9 ; Snake
     "00000000001111111111111111111111110401111111111111111111110000000000",
     "00000000111111111111111111111111114141111111111111111111111100000000",
     "00000011111111111111111111111111111111111111111111111111111111000000",
     "00000111111111111111111111111111111111111111111111111111111111100000",
     "00011111111111111111111111111111111111111111111111111111111111111000",
     "00111111111111111111111111111111111111111111111111111111111111111100",
     "01111111111111111111111111111111111111111111111111111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "00000000000000000000001111111000111111111111111000000000000000000000",
     "00000000000000000000011111111000011111111111111100000000000000000000",        
     "00000000000000000000001111111110000111111111111110000000000000000000",
     "00000000000000000011000111111111000011111111111000100000000000000000",
     "00000000000000001111100011111111110011111111110001111000000000000000",
     "00000000000000111111600061111111106060111111100011111110000000000000",
     "00000000000011111111000001111111100000111100000111111111000000000000",    
     "00000000000111111111064601111111106460111106060111111111110000000000",    // 10 ; Snakes
     "00000000001111111111104011111111110401111100000111111111110000000000",
     "00000000111111111111141411111111114141111114461111111111111100000000",
     "00000011111111111111111111111111111111111144111111111111111111000000",
     "00000111111111111111111111111111111111111414111111111111111111100000",
     "00011111111111111111111111111111111111111111111111111111111111111000",
     "00111111111111111111111111111111111111111111111111111111111111111100",
     "01111111111111111111111111111111111111111111111111111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "00000000000000000000001100111000111111100011111000000000000000000000",
     "00000000000000000000011110011000011111060601111100000000000000000000",        
     "00000000000000000000001111001110000111000001111110000000000000000000",
     "00000000000000000011000111001111000011064601111000100000000000000000",
     "00000000000000001111100011100111110011104011110001111000000000000000",
     "00000000000000111111600061100111106060114111100011111110000000000000",
     "00000000000000001111000001060601100000141400000111111111000000000000",    
     "00000000000110000111064601000001106460111106060111111111110000000000",    // 11 ; More Snakes
     "00000000001111000011104011064601110401111100000111111111100000000000",
     "00000000111111110011141411114111114141111114461111111000000100000000",
     "00000011111111110006011111141411111111111144110606011000001111000000",
     "00000111111111111060011111111111111111111414110000000000111111100000",
     "00011111111111111000441111111111111111111111110646000001111111111000",
     "00111111111111111111144111111111111111111111111040111111111111111100",
     "01111111111111111111141111111111111111111111111414111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "33333333333333333333333333333333333333333333333333333333333333333333",
     "00000000000000000000001111111111111111111111001000000000000000000000",
     "00000000000000000000111111000000000000000111000100000000000000000000",        
     "00000000000000000001111110000000000000000011100010000000000000000000",
     "00000000000000000011111100011111212121210001100011100000000000000000",
     "00000000000000001111111100011110022222111000000111111000000000000000",
     "00000000000000111111111110001000004040111100001111111110000000000000",
     "00000000000011111111111111000001100000111111111111111111000000000000",    
     "00000000000111111111111111100111106460111111111111111111110000000000",    // 12 ; King Snake
     "00000000001111111111111111111111110401111111111111111111110000000000",
     "00000000111111111111111111111111114141111111111111111111111100000000",
     "00000011111111111111111111111111111111111111111111111111111111000000",
     "00000111111111111111111111111111111111111111111111111111111111100000",
     "00011111111111111111111111111111111111111111111111111111111111111000",
     "00111111111111111111111111111111111111111111111111111111111111111100",
     "01111111111111111111111111111111111111111111111111111111111111111110",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
    {"77777777777777777777777777777777777777777777777777777777777777777777",
     "77777777777777777777777777777726222627777777777777777777777777777777",
     "77777777777777777777777777777224626422777777777777777777777777777777",
     "77777777777777777777777777777222222222777777777777777777777777777777",
     "77777777777777777777777777777722666227777777777777777777777777777777",
     "77777777777777777777777772222222222222222277777777777777777777777777",
     "77777777777777777777777222222222222222222222277777777777777777777777",
     "11111111111111111111112222222222222222211222221111111111111111111111",
     "11111111111111111111112222122222222222221122221111111111111111111111",
     "11111111111111111111112221122222222222221222221111111111111111111111",
     "11111111111111111111122221122221111122221122211111111111111111111111",
     "11111111111111111111122211122211111112221166661111111111111111111111",
     "11111111111111111111666611222111111111222166661111111111111111111111",
     "11111111111111111111666611222111111111222111111111111111111111111111",
     "11111111111111111111111111222111111111222111111111111111111111111111",    // 13 ; Angry Men
     "11111111111111111111111112222211111112222211111111111111111111111111",
     "11111111111111111111111122222111111111222221111111111111111111111111",
     "11111111111111111111111111111111111111111111111111111111111111111111",
     "11111111111111111111111111111111111111111111111111111111111111111111",
     "11111111111111111111111111111111111111111111111111111111111111111111",
     "11111111111111111111111111111111111111111111111111111111111111111111",
     "11111111111111111111111111111111111111111111111111111111111111111111",
     "11111111111111111111111111111111111111111111111111111111111111111111"
    },
};

string opponentToString(int opponentnmr) {
    ostringstream os;
    for (int i = 0; i < (int)lengthof(opponentVis[opponentnmr]); i++) {
        for (int j = 0; j < (int)opponentVis[opponentnmr][i].length(); j++) {
            if (opponentVis[opponentnmr][i][j] == '0')
                os << PIXEL_GREEN;
            else if (opponentVis[opponentnmr][i][j] == '1')
                os << PIXEL_WHITE;
            else if (opponentVis[opponentnmr][i][j] == '2')
                os << PIXEL_YELLOW;
            else if (opponentVis[opponentnmr][i][j] == '3')
                os << PIXEL_BLUE;
            else if (opponentVis[opponentnmr][i][j] == '4')
                os << PIXEL_RED;
            else if (opponentVis[opponentnmr][i][j] == '5')
                os << PIXEL_MAGENTA;
            else if (opponentVis[opponentnmr][i][j] == '6')
                os << PIXEL_BLACK;
            else if (opponentVis[opponentnmr][i][j] == '7')
                os << PIXEL_CYAN;
        }
        os << endl;
    }
    return os.str();
}

bool battle(int opponentnmr) {
    bool retreat = false;
    bool giveOptions = true;
    int damage = 0;
    int speed = 0;
    bool attack = true;
    cout << opponentToString(opponentnmr) << endl;
    typeOut("You encounter an enemy! [Press Enter]");
    getCin();
    int opponenthealth = opponents[opponentnmr].hp;
    while (!retreat && opponenthealth > 0 && health > 0) {
        attack = true;
        if (giveOptions)
            clear();
        if (damage != 0) { damage -= 1; typeOut(colored("Extra Damage", "text", "red")); }
        if (speed != 0) { speed -= 1; typeOut(colored("Extra Speed", "text", "cyan")); }
        typeOut("Your Health: " + colored(to_string(health), "text", "red") + "\nEnemy Health: " + colored(to_string(opponenthealth), "text", "red"));
        map<string, string> options;
        map<string, string> specialOptions;
        specialOptions.insert(pair<string, string>("1a", "Attack Enemy"));
        specialOptions.insert(pair<string, string>("2p", "Use A Potion"));
        specialOptions.insert(pair<string, string>("3r", "Retreat"));
        string input = optionsNav(options, specialOptions, "Fight");
        if (input == "r") {
            retreat = true;
            attack = false;
            typeOut("You retreat cowardly out of fear... you feel ashamed.");
            stage = prevstage;
            if (sanity - 2 > 0) {
                sanity -= 2;
                typeOut("Your " + colored("sanity", "text", "red") + " was depleted by 2. [Press Enter]");
            }
            else {
                sanity = 0;
                respawn = true;
                typeOut("Your " + colored("sanity", "text", "red") + " was drained to 0. You died of insanity. [Press Enter]");
            }
            getCin();
        }
        else if (input == "p") {
            bool exit = false;
            while (!exit) {
                typeOut("What " + colored("potion", "text", "green") + " do you want to use?");
                string input2;
                int iteration = 0;
                bool rerender = true;
                map<string, string> specialOptions;
                specialOptions.insert(pair<string, string>("1", "Health Potion"));
                specialOptions.insert(pair<string, string>("2", "Damage Potion"));
                specialOptions.insert(pair<string, string>("3", "Speed Potion"));
                specialOptions.insert(pair<string, string>("4e", "Exit"));
                cout << "Use up and down arrow keys to navigate, right arrow key to pick." << endl;
                set_cursor(false);
                do {
                    io.check_sync();
                    if (rerender) {
                        rerender = false;
                        int i = 0;
                        for (auto const& p : specialOptions) {
                            if (iteration == i)
                                cout << colored("Pot >\t" + p.second, "text", "yellow") << endl;
                            else if (i == 0)
                                cout << colored("     \t" + p.second, "text", "magenta") << endl;
                            else if (i == 1)
                                cout << colored("     \t" + p.second, "text", "red") << endl;
                            else if (i == 2)
                                cout << colored("     \t" + p.second, "text", "cyan") << endl;
                            else
                                cout << colored("     \t" + p.second, "text", "blue") << endl;
                            i++;
                        }
                        goBack(specialOptions.size());
                    }
                    if (io.pressed[K_UP])
                        if (iteration > 0) {
                            iteration--;
                            rerender = true;
                        }
                    if (io.pressed[K_DOWN])
                        if (iteration < (int)specialOptions.size() - 1) {
                            iteration++;
                            rerender = true;
                        }
                } while (!io.pressed[K_RIGHT]);
                for (int i = 0; i < (int)specialOptions.size(); i++)
                    cout << endl;
                set_cursor(true);
                int i = 0;
                for (auto const& p : specialOptions) {
                    if (iteration == i)
                        input2 = toString(p.first[p.first.length() - 1]);
                    i++;
                }
                if (isdigit(input2[0])) {
                    int inputint = input2[0] - '0';
                    if (inputint > 0 && inputint <= 3) {
                        if (inputint == 1) {
                            typeOut("You went ahead and took a " + colored("Health Potion", "text", "magenta") + ".");
                            if (health + 70 >= 100) {
                                health = 100;
                                typeOut("Your " + colored("Health", "text", "magenta") + " was set to 100.");
                            }
                            else {
                                health += 70;
                                typeOut("Your " + colored("Health", "text", "magenta") + " got increased by 70.");
                            }
                        }
                        if (inputint == 2) {
                            typeOut("You went ahead and took a " + colored("Damage Potion", "text", "red") + ".");
                            damage += 3;
                            typeOut("You gained the " + colored("Damage", "text", "red") + " effect for 3 rounds.");
                        }
                        if (inputint == 3) {
                            typeOut("You went ahead and took a " + colored("Speed Potion", "text", "cyan") + ".");
                            speed += 2;
                            typeOut("You gained the " + colored("Speed", "text", "cyan") + " effect for 2 rounds.");
                        }
                        exit = true;
                    }
                    else {
                        typeOut("The potion number " + to_string(inputint) + " is not available!\n");
                    }
                }
                else {
                    typeOut("'" + toString(input2[0]) + "' is not a valid option.");
                }
            }
        }
        else if (input == "a") {
            double dmgmul = 1.0;
            if (damage != 0)
                dmgmul = 1.6;
            typeOut("You prepare an attack! Press the up arrow key to strike!");
            set_cursor(false);
            ostringstream os;
            for (int i = 0; i < (int)lengthof(battleVis); i++) {
                os << "\n";
                for (int j = 0; j < (int)battleVis[i].length(); j++) {
                    if (battleVis[i][j] == '0')
                        os << PIXEL_GREEN;
                    else if (battleVis[i][j] == '1')
                        os << PIXEL_WHITE;
                    else if (battleVis[i][j] == '2')
                        os << PIXEL_YELLOW;
                    else if (battleVis[i][j] == '3')
                        os << PIXEL_CYAN;
                    else if (battleVis[i][j] == '4')
                        os << PIXEL_BLUE;
                    else if (battleVis[i][j] == '5')
                        os << PIXEL_MAGENTA;
                    else if (battleVis[i][j] == '6')
                        os << PIXEL_RED;
                    else if (battleVis[i][j] == '7')
                        os << PIXEL_BLACK;
                }
            }
            cout << os.str() << endl;
            bool forward = true;
            double progress = 0.0;
            do {
                io.check();
                double barWidth = 70;
                std::cout << "[";
                double pos = barWidth * progress;
                for (int i = 0; i < (int)barWidth; i++) {
                    if (i < pos - 1) std::cout << " ";
                    else if (i > pos) std::cout << " ";
                    else std::cout << "^";
                }
                cout << "] " << "\r";
                cout.flush();

                if (progress >= 1) { forward = false; }
                else if (progress <= 0) { forward = true; }
                if (forward) { progress += 0.01; }
                else { progress -= 0.01; }
                MSDelay(10);       
            } while (!io.pressed[K_UP]);
            int dmg = (int)((playerDmg - abs(50 - 100 * progress)) * dmgmul + 4);
            dmg -= (int)(dmg * ((double)opponents[opponentnmr].resistance / 100));
            if (dmg < 0) { dmg = 0; }
            if (opponenthealth - dmg < 0) { opponenthealth = 0; }
            else { opponenthealth -= dmg; }
            set_cursor(true);
            typeOut("\nYou struck the enemy for " + colored(to_string(dmg), "text", "red") + " damage! It's now on " + colored(to_string(opponenthealth), "text", "red") + " health!");
            if (opponenthealth == 0) {
                attack = false;
                int coinreward = (my_rand(opponents[opponentnmr].coinrewardmax - opponents[opponentnmr].coinrewardmin + 1)) + opponents[opponentnmr].coinrewardmin;
                coins += coinreward;
                typeOut("You defeated the enemy! You gained " + colored(to_string(coinreward), "text", "yellow") + " coins! [Press Enter]");
                getCin();
            }
            giveOptions = true;
        }
        else {
            typeOut("I am not sure what you meant with '" + input + "', but its not a valid option.\n");
            attack = false;
        }
        if (attack) {
            typeOut("Remember to use the arrow Keys!");
            cout << "Enemy attack in 3... " << flush;
            MSDelay(1000);
            cout << "2... " << flush;
            MSDelay(1000);
            cout << "1... " << flush;
            MSDelay(1000);
            cout << "\r" << flush;
            set_cursor(false); // hide cursor
            int attackFrames = opponents[opponentnmr].attackdurance;
            bool exitAttack = false;
            const int rows = Y_SIZE;
            const int charPerRow = X_SIZE;
            int playerX = 50;
            int playerY = 4;
            int speedmul = 1;
            if (speed != 0)
                speedmul = 2;
            const int invincibilityFramesMax = 30;
            const int invincibilityFrameChange = 2;
            int invincibilityFrames = 0;
            if (opponents[opponentnmr].type == 0) { // is attack type spikes?
                // map<int, int> spikesX;
                vector<int> spikesX(charPerRow); 
                vector<int> spikesY(rows); 
                vector<int> spikesY2(rows); 
                vector<int> spikesY3(rows); 
                int chanceofspike = 1;
                if (opponents[opponentnmr].difficulty > 2) // add 50% chance if difficulty is under or equal to 2
                    int chanceofspikeX = 0;
                const int spikeMove = 2;
                const int maxSpikeCooldown = 10 * opponents[opponentnmr].difficulty;
                const int ammountSpikesX = 4 * opponents[opponentnmr].difficulty;
                const int ammountSpikesY = opponents[opponentnmr].difficulty;
                const int afkSpikeDelayMaxX = 80 / opponents[opponentnmr].difficulty;
                const int afkSpikeDelayMaxY = 400 / opponents[opponentnmr].difficulty;
                int spikes2Countdown = 100;
                int spikes3Countdown = 200;
                int afkSpikeDelayX = 0;
                int afkSpikeDelayY = 0;
                int start = clock();
                int dt = 3000 / FPS;
                vector<vector<int>> spikesVis(charPerRow+1, vector<int>(rows));
                while (attackFrames != 0 && health > 0) {
                    for (auto&i:spikesVis)fill(i.begin(),i.end(),0);
                    if (invincibilityFrames > 0) // substract invincibility
                        invincibilityFrames -= 1;
                    if (spikes2Countdown > 0)
                        spikes2Countdown -= 1;
                    if (spikes3Countdown > 0)
                        spikes3Countdown -= 1;
                    attackFrames -= 1;
                    // anti AFK for levels above 1 ; if the delay is 0
                    if (afkSpikeDelayX == 0) {
                        afkSpikeDelayX = (my_rand(afkSpikeDelayMaxX));
                        if (opponents[opponentnmr].difficulty > 1) {
                            int point1 = playerX;
                            if (spikesX[point1] == 0) // is this spike already claimed?
                                spikesX[point1] = rows * spikeMove + (my_rand(afkSpikeDelayMaxX)) * spikeMove; // Fall ; with custom delay
                        }
                    }
                    else
                        afkSpikeDelayX -= 1;
                    if (afkSpikeDelayY == 0) {
                        afkSpikeDelayY = (my_rand(afkSpikeDelayMaxY));
                        if (opponents[opponentnmr].difficulty > 1) {
                            int point2 = playerY;
                            if (spikesY[point2] == 0) // is this spike already claimed?
                                spikesY[point2] = charPerRow * spikeMove; // no delay here, just immidiate fall
                        }
                    }
                    else
                        afkSpikeDelayY -= 1;
                    // spikes X
                    if (my_rand(chanceofspike) == 0) { // Is there going to be spike(s) summoned?
                        for (int i = 0; i < ammountSpikesX; i++) {
                            int point3 = my_rand(charPerRow); // Grab random spike pos
                            if (spikesX[point3] == 0) // is this spike already claimed?
                                spikesX[point3] = rows * spikeMove + (my_rand(maxSpikeCooldown)) * spikeMove;
                        }
                    }
                    // spikes Y
                    if (opponents[opponentnmr].difficulty > 1) {
                        for (int i = 0; i < ammountSpikesY; i++) {
                            int point4 = my_rand(rows); // Grab random spike pos
                            if (spikesY[point4] == 0) // is this spike already claimed?
                                spikesY[point4] = charPerRow * spikeMove;
                        }
                    }
                    if (opponents[opponentnmr].difficulty > 2 && spikes2Countdown == 0) {
                        for (int i = 0; i < ammountSpikesY; i++) {
                            int point5 = my_rand(rows); // Grab random spike pos
                            if (spikesY2[point5] == 0) // is this spike already claimed?
                                spikesY2[point5] = charPerRow * spikeMove;
                        }
                    }
                    if (opponents[opponentnmr].difficulty > 3 && spikes3Countdown == 0) {
                        for (int i = 0; i < ammountSpikesY; i++) {
                            int point6 = my_rand(rows); // Grab random spike pos
                            if (spikesY3[point6] == 0) // is this spike already claimed?
                                spikesY3[point6] = charPerRow * spikeMove;
                        }
                    }
                    // spike update
                    // X
                    for (int x=0; x<charPerRow; x++)
                    {
                        if (spikesX[x] > 0) {
                            int value = spikesX[x] / spikeMove;
                            if (value > rows - 1)
                                spikesVis[x][0] = 1;
                            else
                                spikesVis[x][rows - value] = 1;
                            spikesX[x] -= 1;
                        }
                    }
                    // Y 1
                    if (opponents[opponentnmr].difficulty > 1) {
                        for (int x=0;x<rows;x++)
                        {
                            if (spikesY[x] > 0) {
                                int value = spikesY[x] / spikeMove;
                                if (value > charPerRow - 1)
                                    spikesVis[0][x] = 1;
                                else
                                    spikesVis[charPerRow - value][x] = 1;
                                spikesY[x] -= 1;
                            }
                        }
                    }
                    // Y 2
                    if (opponents[opponentnmr].difficulty > 2) {
                        for (int x=0;x<rows;x++)
                        {
                            if (spikesY2[x] > 0) {
                                int value = spikesY2[x] / spikeMove;
                                if (value > charPerRow - 1)
                                    spikesVis[0][x] = 1;
                                else
                                    spikesVis[charPerRow - value][x] = 1;
                                spikesY[x] -= 1;
                            }
                        }
                    }
                    // Y 3
                    if (opponents[opponentnmr].difficulty > 3) {
                        for (int x=0;x<rows;x++)
                        {
                            if (spikesY3[x] > 0) {
                                int value = spikesY3[x] / spikeMove;
                                if (value > charPerRow - 1)
                                    spikesVis[0][x] = 1;
                                else
                                    spikesVis[charPerRow - value][x] = 1;
                                spikesY[x] -= 1;
                            }
                        }
                    }
                    if (spikesVis[playerX][playerY] == 1 && attackFrames > 0) // hit?
                        if (invincibilityFrames == 0) { // player must not be invincible
                            invincibilityFrames = invincibilityFramesMax; // hit
                            int attackDamage = opponents[opponentnmr].attackdmg;
                            if (opponents[opponentnmr].dmgrange != 0) { // random damage range?
                                attackDamage += (my_rand(opponents[opponentnmr].dmgrange + 1)) - opponents[opponentnmr].dmgrange / 2; // pick number between 1 - damage range, subtract by damage range / 2
                            }
                            if (health - attackDamage < 0)
                                health = 0;
                            else
                                health -= attackDamage; // substract health
                        }
                    // screen refresh
                    for (int i = 0; i < rows; i++) { // for every row
                        for (int j = 0; j < charPerRow; j++) { // for every char in row
                            if (i == playerY && j == playerX) {
                                // invincibility calculation
                                if (invincibilityFrames % invincibilityFrameChange > 0)
                                    cout << PIXEL_RED; // player got hurt and is invincible
                                else
                                    cout << PIXEL_CYAN;
                            }
                            else if (spikesVis[j][i] == 1)
                                cout << PIXEL_WHITE;
                            else
                                cout << PIXEL_GREEN;
                        }
                        cout << endl;
                    }
                    io.check();
                    if (io.pressed[K_UP] && playerY != 0 && attackFrames > 0)
                        playerY -= 1;
                    if (io.pressed[K_DOWN] && playerY != rows - 1 && attackFrames > 0)
                        playerY += 1;
                    if (io.pressed[K_LEFT] && playerX != 0 && attackFrames > 0)
                        playerX -= 1 * speedmul;
                    if (io.pressed[K_RIGHT] && playerX != charPerRow - 1 && attackFrames > 0)
                        playerX += 1 * speedmul;
                    // render health bar
                    cout << endl;
                    for (int i = 0; i < (int)charPerRow / 2 - 3; i++)
                        cout << " ";
                    cout << "HEALTH" << endl;
                    if (health > 0) {
                        double healthBar = 100.0 / defHealth * health / 100.0;
                        double barWidth = charPerRow;
                        double pos1 = barWidth * healthBar;
                        for (int i = 0; i < (int)barWidth; i++) {
                            if (i <= pos1) cout << PIXEL_RED;
                            else std::cout << PIXEL_WHITE;
                        }
                        cout << " " << health << " %  " << endl;
                    }
                    else {
                        for (int i = 0; i < 101; i++)
                            cout << PIXEL_WHITE;
                        cout << " 0 %    " << endl;
                    }
                    goBack(rows + 3);
                    MSDelay(dt);
                }
                for (int i = 0; i < rows + 3; i++) // for every row + healthbar
                    cout << endl;
            }
            if (opponents[opponentnmr].type == 1) { // is attack type bombs?
                map<int, int[2]> bombs;
                map<int, int> snake;
                int snakepos = 0;
                map<int, int> snake2;
                int snake2pos = 0;
                int snake2activate = 50;
                int snakedurance = 40;
                if (opponents[opponentnmr].difficulty < 2)
                    snakedurance = 20; // snake is less thicc
                if (opponents[opponentnmr].difficulty > 4)
                    snakedurance = 10 * opponents[opponentnmr].difficulty; // snake is more thicc
                int snakemove = 2;
                if (opponents[opponentnmr].difficulty > 2)
                    snakemove = 1; // snake moves faster
                int chanceofbomb = 2;
                if (opponents[opponentnmr].difficulty > 2)
                    chanceofbomb = 5 - opponents[opponentnmr].difficulty;
                if (opponents[opponentnmr].difficulty > 5)
                    chanceofbomb = 1;
                int bombMove = 4;
                if (opponents[opponentnmr].difficulty > 2) // bombs move faster
                    bombMove = 3;
                if (opponents[opponentnmr].difficulty > 4) // bombs move faster
                    bombMove = 2;
                int explodeMove = 3;
                if (opponents[opponentnmr].difficulty > 1) // explode move faster
                    explodeMove = 2;
                if (opponents[opponentnmr].difficulty > 4) // explode move faster
                    explodeMove = 1;
                int maxBombCooldown = 10 * opponents[opponentnmr].difficulty;
                if (opponents[opponentnmr].difficulty > 4) // bombs move faster
                    maxBombCooldown = 40;
                bool upgradedBombs = false;
                if (opponents[opponentnmr].difficulty > 2)
                    upgradedBombs = true;
                int ammountbombs = 5;
                if (opponents[opponentnmr].difficulty > 2)
                    ammountbombs = 6;
                if (opponents[opponentnmr].difficulty > 4)
                    ammountbombs = opponents[opponentnmr].difficulty;
                int start = clock();
                int dt = 1000 / FPS;
                while (attackFrames > 0 && health > 0) {
                    map<int, map<int, int>> bombsVis;
                    if (invincibilityFrames > 0) // substract invincibility
                        invincibilityFrames -= 1;
                    attackFrames -= 1;
                    // bombs
                    if (my_rand(chanceofbomb) == 0) { // Is there going to be a bomb summoned?
                        for (int i = 0; i < ammountbombs; i++) {
                            int point = my_rand(charPerRow); // Grab random bomb pos
                            if (bombs[point][0] == 0) { // is this bomb already claimed?
                                bombs[point][0] = rows * bombMove + (my_rand(maxBombCooldown)) * bombMove;
                                bombs[point][1] = 0 - (rows - 1 - my_rand((int)(rows - rows / 3)));
                            }
                        }
                    }
                    // upgraded    id   not upgraded
                    // 000000000        000000000
                    // 000000000        000000000
                    // 000010000    0   000010000
                    // 000000000        000000000
                    // 000000000        000000000
                    
                    // 000000000        000000000
                    // 000111000        000010000
                    // 000101000    1   000101000
                    // 000111000        000010000
                    // 000000000        000000000
                    
                    // 000111000        000000000
                    // 001101100        000111000
                    // 011000110    2   001000100
                    // 001101100        000111000
                    // 000111000        000000000

                    // 0000000000000        0000000000000
                    // 0000111110000        0000011100000
                    // 0001100011000        0000100010000
                    // 0011000001100    3   0001000001000
                    // 0001100011000        0000100010000
                    // 0000111110000        0000011100000
                    // 0000000000000        0000000000000

                    // bomb update
                    for (auto const& x : bombs)
                    {
                        if (bombs[x.first][0] > 0) {
                            int value = bombs[x.first][0] / bombMove;
                            if (bombs[x.first][1] < 0){ 
                                if (value == 0 - bombs[x.first][1])
                                    bombs[x.first][1] = 0;
                                else {
                                    if (value > rows - 1)
                                        bombsVis[x.first][0] = 1;
                                    else
                                        bombsVis[x.first][rows - value] = 1;
                                }
                            }
                            else if (bombs[x.first][1] >= 0) {
                                if (bombs[x.first][1] - 1 > 0)
                                    if (bombs[x.first][1] - 1 < 3)
                                        value += bombs[x.first][1] - 1;
                                    else
                                        value += 2;
                                int value2 = (int)bombs[x.first][1] / explodeMove;
                                if (value2 == 0)
                                    bombsVis[x.first][value] = 1;
                                if (value2 == 1) {
                                    if (upgradedBombs) {
                                        bombsVis[x.first - 1][value - 1] = 1;
                                        bombsVis[x.first][value - 1] = 1;
                                        bombsVis[x.first + 1][value - 1] = 1;
                                        bombsVis[x.first - 1][value] = 1;
                                        bombsVis[x.first + 1][value] = 1;
                                        bombsVis[x.first - 1][value + 1] = 1;
                                        bombsVis[x.first][value + 1] = 1;
                                        bombsVis[x.first + 1][value + 1] = 1;
                                    }
                                    else {
                                        bombsVis[x.first][value - 1] = 1;
                                        bombsVis[x.first - 1][value] = 1;
                                        bombsVis[x.first + 1][value] = 1;
                                        bombsVis[x.first][value + 1] = 1;
                                    }
                                }
                                if (value2 == 2) {
                                    if (upgradedBombs) {
                                        bombsVis[x.first - 1][value - 2] = 1;
                                        bombsVis[x.first][value - 2] = 1;
                                        bombsVis[x.first + 1][value - 2] = 1;
                                        bombsVis[x.first - 2][value - 1] = 1;
                                        bombsVis[x.first - 1][value - 1] = 1;
                                        bombsVis[x.first + 1][value - 1] = 1;
                                        bombsVis[x.first + 2][value - 1] = 1;
                                        bombsVis[x.first + 3][value] = 1;
                                        bombsVis[x.first + 2][value] = 1;
                                        bombsVis[x.first - 2][value] = 1;
                                        bombsVis[x.first - 3][value] = 1;
                                        bombsVis[x.first - 2][value + 1] = 1;
                                        bombsVis[x.first - 1][value + 1] = 1;
                                        bombsVis[x.first + 1][value + 1] = 1;
                                        bombsVis[x.first + 2][value + 1] = 1;
                                        bombsVis[x.first - 1][value + 2] = 1;
                                        bombsVis[x.first][value + 2] = 1;
                                        bombsVis[x.first + 1][value + 2] = 1;
                                    }
                                    else {
                                        bombsVis[x.first - 1][value - 1] = 1;
                                        bombsVis[x.first][value - 1] = 1;
                                        bombsVis[x.first + 1][value - 1] = 1;
                                        bombsVis[x.first - 2][value] = 1;
                                        bombsVis[x.first + 2][value] = 1;
                                        bombsVis[x.first - 1][value + 1] = 1;
                                        bombsVis[x.first][value + 1] = 1;
                                        bombsVis[x.first + 1][value + 1] = 1;
                                    }
                                }
                                if (value2 == 3) {
                                    if (upgradedBombs) {
                                        bombsVis[x.first - 2][value - 2] = 1;
                                        bombsVis[x.first - 1][value - 2] = 1;
                                        bombsVis[x.first][value - 2] = 1;
                                        bombsVis[x.first + 1][value - 2] = 1;
                                        bombsVis[x.first + 2][value - 2] = 1;
                                        bombsVis[x.first - 3][value - 1] = 1;
                                        bombsVis[x.first - 2][value - 1] = 1;
                                        bombsVis[x.first + 2][value - 1] = 1;
                                        bombsVis[x.first + 3][value - 1] = 1;
                                        bombsVis[x.first + 4][value] = 1;
                                        bombsVis[x.first + 3][value] = 1;
                                        bombsVis[x.first - 3][value] = 1;
                                        bombsVis[x.first - 4][value] = 1;
                                        bombsVis[x.first - 3][value + 1] = 1;
                                        bombsVis[x.first - 2][value + 1] = 1;
                                        bombsVis[x.first + 2][value + 1] = 1;
                                        bombsVis[x.first + 3][value + 1] = 1;
                                        bombsVis[x.first - 2][value + 2] = 1;
                                        bombsVis[x.first - 1][value + 2] = 1;
                                        bombsVis[x.first][value + 2] = 1;
                                        bombsVis[x.first + 1][value + 2] = 1;
                                        bombsVis[x.first + 2][value + 2] = 1;
                                    }
                                    else {
                                        bombsVis[x.first - 1][value - 2] = 1;
                                        bombsVis[x.first][value - 2] = 1;
                                        bombsVis[x.first + 1][value - 2] = 1;
                                        bombsVis[x.first - 2][value - 1] = 1;
                                        bombsVis[x.first + 2][value - 1] = 1;
                                        bombsVis[x.first - 3][value] = 1;
                                        bombsVis[x.first + 3][value] = 1;
                                        bombsVis[x.first - 2][value + 1] = 1;
                                        bombsVis[x.first + 2][value + 1] = 1;
                                        bombsVis[x.first - 1][value + 2] = 1;
                                        bombsVis[x.first][value + 2] = 1;
                                        bombsVis[x.first + 1][value + 2] = 1;
                                    }
                                }
                                bombs[x.first][1] += 1;
                            }
                            bombs[x.first][0] -= 1;
                        }
                    }
                    // for levels above 1, bottom row snake
                    if (snakemove > 0) {
                        if (snake2activate > 0)
                            snake2activate -= 1; // frame countdown for snake 2
                        // summon new part of body
                        int value = snakepos / snakemove;
                        snake[value] = snakedurance;
                        if (snakepos >= charPerRow * snakemove)
                            snakepos = 0;
                        else
                            snakepos += 1;
                        if (snake2activate == 0) {
                            int value2 = snake2pos / snakemove;
                            snake2[value2] = snakedurance;
                            if (snake2pos <= 0)
                                snake2pos = charPerRow * snakemove;
                            else
                                snake2pos -= 1;
                        }
                        // update and render snake body
                        for (auto const& x : snake) {
                            if (snake[x.first] > 0) {
                                bombsVis[x.first][rows - 1] = 1;
                                snake[x.first] -= 1;
                            }
                        }
                        if (snake2activate == 0) {
                            for (auto const& x : snake2) {
                                if (snake2[x.first] > 0) {
                                    bombsVis[x.first][rows - 2] = 1;
                                    snake2[x.first] -= 1;
                                }
                            }
                        }
                    }
                    if (bombsVis[playerX][playerY] == 1 && attackFrames > 0) // hit?
                        if (invincibilityFrames == 0) { // player must not be invincible
                            invincibilityFrames = invincibilityFramesMax; // hit
                            int attackDamage = opponents[opponentnmr].attackdmg;
                            if (opponents[opponentnmr].dmgrange != 0) { // random damage range?
                                attackDamage += (my_rand(opponents[opponentnmr].dmgrange + 1)) - opponents[opponentnmr].dmgrange / 2; // pick number between 1 - damage range, subtract by damage range / 2
                            }
                            if (health - attackDamage < 0)
                                health = 0;
                            else
                                health -= attackDamage; // substract health
                        }
                    // screen refresh
                    for (int i = 0; i < rows; i++) { // for every row
                        for (int j = 0; j < charPerRow; j++) { // for every char in row
                            if (i == playerY && j == playerX) {
                                // invincibility calculation
                                if (invincibilityFrames % invincibilityFrameChange > 0)
                                    cout << PIXEL_RED; // player got hurt and is invincible
                                else
                                    cout << PIXEL_CYAN;
                            }
                            else if (bombsVis[j][i] == 1)
                                cout << PIXEL_WHITE;
                            else
                                cout << PIXEL_GREEN;
                        }
                        cout << "\n";
                    }
                    io.check();
                    if (io.pressed[K_UP] && playerY != 0 && attackFrames > 0)
                        playerY -= 1;
                    if (io.pressed[K_DOWN] && playerY != rows - 1 && attackFrames > 0)
                        playerY += 1;
                    if (io.pressed[K_LEFT] && playerX != 0 && attackFrames > 0)
                        playerX -= 1 * speedmul;
                    if (io.pressed[K_RIGHT] && playerX != charPerRow - 1 && attackFrames > 0)
                        playerX += 1 * speedmul; 
                    // render health bar
                    cout << endl;
                    for (int i = 0; i < (int)charPerRow / 2 - 3; i++)
                        cout << " ";
                    cout << "HEALTH" << endl;
                    if (health > 0) {
                        double healthBar = 100.0 / defHealth * health / 100.0;
                        double barWidth = charPerRow;
                        double pos1 = barWidth * healthBar;
                        for (int i = 0; i < (int)barWidth; i++) {
                            if (i <= pos1) cout << PIXEL_RED;
                            else std::cout << PIXEL_WHITE;
                        }
                        cout << " " << health << " %  " << endl;
                    }
                    else {
                        for (int i = 0; i < 101; i++)
                            cout << PIXEL_WHITE;
                        cout << " 0 %    " << endl;
                    }
                    goBack(rows + 3);
                    MSDelay(dt);
                }
                for (int i = 0; i < rows + 3; i++) // for every row + healthbar
                    cout << endl;
            }
            if (opponents[opponentnmr].type == 2) { // is attack type snakes? 
                map<int, map<int, int>> snakes; // snakes[X][Y] -> duration
                const int randomSnakeCountdown = 100;
                const int randomSnakeMoveMax = 3;
                int randomSnakeIgnorance = 3;
                if (opponents[opponentnmr].difficulty > 0)
                    randomSnakeIgnorance = 2;
                int positionsX[charPerRow]{};
                int snakeMoveX[charPerRow]{};
                int directionsX[charPerRow]{};
                for (int i = 0; i < (int)lengthof(positionsX); i++) {
                    positionsX[i] = 0 - (my_rand(randomSnakeCountdown));
                    if (my_rand(randomSnakeIgnorance) == 0)
                        snakeMoveX[i] = 1 + (my_rand(randomSnakeMoveMax * 5));
                    else
                        snakeMoveX[i] = 0;
                    directionsX[i] = my_rand(2);
                }
                int positionsY[rows]{};
                int snakeMoveY[rows]{};
                int directionsY[rows]{};
                for (int i = 0; i < (int)lengthof(positionsY); i++) {
                    positionsY[i] = 0 - (my_rand(randomSnakeCountdown));
                    snakeMoveY[i] = 1 + (my_rand(randomSnakeMoveMax));
                    directionsY[i] = my_rand(2);
                }
                const int snakeduranceX = 1 * opponents[opponentnmr].difficulty;
                const int snakeduranceY = 5 * opponents[opponentnmr].difficulty;
                int start = clock();
                int dt = 1000 / FPS;
                while (attackFrames != 0 && health > 0) {
                    if (invincibilityFrames > 0) // substract invincibility
                        invincibilityFrames -= 1;
                    attackFrames -= 1;
                    // update snake countdowns and summon new parts for X
                    for (int i = 0; i < (int)lengthof(positionsX); i++) {
                        if (snakeMoveX[i] > 0) {
                            if (positionsX[i] < 0) {
                                positionsX[i] += 1;
                            }
                            else {
                                if (directionsX[i] == 0) {
                                    int value = positionsX[i] / snakeMoveX[i];
                                    snakes[i][value] = snakeduranceX * snakeMoveX[i];
                                    if (positionsX[i] >= rows * snakeMoveX[i])
                                        positionsX[i] = 0;
                                    else
                                        positionsX[i] += 1;
                                }
                                else {
                                    int value = positionsX[i] / snakeMoveX[i];
                                    snakes[i][value] = snakeduranceX * snakeMoveX[i];
                                    if (positionsX[i] <= 0)
                                        positionsX[i] = rows * snakeMoveX[i];
                                    else
                                        positionsX[i] -= 1;
                                }
                            }
                        }
                    }
                    // update snake countdowns and summon new parts for Y
                    for (int i = 0; i < (int)lengthof(positionsY); i++) {
                            if (positionsY[i] < 0) {
                                positionsY[i] += 1;
                            }
                            else {
                                if (directionsY[i] == 0) {
                                    int value = positionsY[i] / snakeMoveY[i];
                                    snakes[value][i] = snakeduranceY * snakeMoveY[i];
                                    if (positionsY[i] >= charPerRow * snakeMoveY[i])
                                        positionsY[i] = 0;
                                    else
                                        positionsY[i] += 1;
                                }
                                else {
                                    int value = positionsY[i] / snakeMoveY[i];
                                    snakes[value][i] = snakeduranceY * snakeMoveY[i];
                                    if (positionsY[i] <= 0)
                                        positionsY[i] = charPerRow * snakeMoveY[i];
                                    else
                                        positionsY[i] -= 1;
                                }
                            }
                    }
                    if (snakes[playerX][playerY] > 0 && attackFrames > 0) // hit?
                        if (invincibilityFrames == 0) { // player must not be invincible
                            invincibilityFrames = invincibilityFramesMax; // hit
                            int attackDamage = opponents[opponentnmr].attackdmg;
                            if (opponents[opponentnmr].dmgrange != 0) { // random damage range?
                                attackDamage += (my_rand(opponents[opponentnmr].dmgrange + 1)) - opponents[opponentnmr].dmgrange / 2; // pick number between 1 - damage range, subtract by damage range / 2
                            }
                            if (health - attackDamage < 0)
                                health = 0;
                            else
                                health -= attackDamage; // substract health
                        }
                    // screen refresh
                    for (int i = 0; i < rows; i++) { // for every row
                        for (int j = 0; j < charPerRow; j++) { // for every char in row
                            if (i == playerY && j == playerX) {
                                // invincibility calculation
                                if (invincibilityFrames % invincibilityFrameChange > 0)
                                    cout << PIXEL_RED; // player got hurt and is invincible
                                else
                                    cout << PIXEL_CYAN;
                            }
                            else if (snakes[j][i] > 0) {
                                cout << PIXEL_WHITE;
                                snakes[j][i] -= 1;
                            }
                            else
                                cout << PIXEL_GREEN;
                        }
                        cout << "\n";
                    }
                    io.check();
                    if (io.pressed[K_UP] && playerY != 0 && attackFrames > 0)
                        playerY -= 1;
                    if (io.pressed[K_DOWN] && playerY != rows - 1 && attackFrames > 0)
                        playerY += 1;
                    if (io.pressed[K_LEFT] && playerX != 0 && attackFrames > 0)
                        playerX -= 1 * speedmul;
                    if (io.pressed[K_RIGHT] && playerX != charPerRow - 1 && attackFrames > 0)
                        playerX += 1 * speedmul;
                    // render health bar
                    cout << endl;
                    for (int i = 0; i < (int)charPerRow / 2 - 3; i++)
                        cout << " ";
                    cout << "HEALTH" << endl;
                    if (health > 0) {
                        double healthBar = 100.0 / defHealth * health / 100.0;
                        double barWidth = charPerRow;
                        double pos1 = barWidth * healthBar;
                        for (int i = 0; i < (int)barWidth; i++) {
                            if (i <= pos1) cout << PIXEL_RED;
                            else std::cout << PIXEL_WHITE;
                        }
                        cout << " " << health << " %  " << endl;
                    }
                    else {
                        for (int i = 0; i < 101; i++)
                            cout << PIXEL_WHITE;
                        cout << " 0 %    " << endl;
                    }
                    goBack(rows + 3);
                    
                    MSDelay(dt);
                }
                for (int i = 0; i < rows + 3; i++) // for every row + healthbar
                    cout << endl;
            }
            set_cursor(true);
            if (health == 0) {
                typeOut("You died in the fight, knowing you tried. [Press Enter]");
                respawn = true;
            }
            else
                typeOut("The opponent finished attacking. [Press Enter]");
            getCin();
            giveOptions = true;
        }
    }
    if (opponenthealth > 0)
        return false;
    return true;
}

int possibleEncounters[lengthof(stages)][4] = {
//  {opponents, possibility of encounter}
    {1, 1, 2, 0},   // 0
    {1, 1, 2, 0},   // 1
    {1, 1, 2, 7},   // 2
    {1, 1, 2, 6},   // 3
    {1, 1, 2, 6},   // 4
    {1, 2, 3, 6},   // 5
    {2, 2, 3, 6},   // 6
    {2, 2, 3, 5},   // 7
    {3, 3, 5, 5},   // 8
    {3, 3, 5, 5},   // etc
    {3, 5, 5, 5},
    {3, 5, 5, 5},
    {3, 5, 5, 5},
    {5, 5, 6, 5},
    {5, 5, 6, 5},
    {5, 5, 6, 5},
    {5, 6, 6, 5},
};

void possibleEncounter() {
    if (possibleEncounters[stage][lengthof(possibleEncounters[stage]) - 1] != 0)
        if (my_rand(possibleEncounters[stage][lengthof(possibleEncounters[stage]) - 1]) == 0 )
            battle(possibleEncounters[stage][my_rand((lengthof(possibleEncounters[stage])-1))]);
}

map<int, int> stageEncounters { // always add 1 to enemy number, tutorial = 0 but we type in 1.
    {1, 1},             // In stage 1 ; Tutorial     
    {10, 5},            // In stage 10 ; duelist
    {15, 13},           // In stage 15 ; Angry man
};

map<int, string> stageDialogue{
    {1, "On your way you encounter an old man who wants to help you on your journey. He says:\n'You will face enemies in your way! Let me train you.'\nYou accept. He states:\n'Use me as a training dummy. \nRemember: The fight only ends when I am " + colored("defeated", "text", "green") + ", you are defeated or if you " + colored("retreat", "text", "red") + ".\nYou can " + colored("attack", "text", "green") + " me by choosing the attack option then to strike as much to the middle as possible.\nAfter your turn I will attack, try to dodge my " + colored("spikes", "text", "red") + "!\nUse your arrow keys to " + colored("dodge", "text", "green") + ".'"},
    {10, "You spot a what seems to be angry duelist, because his opponent didn't show up to the fight.\nYou can tell he is hungry for a fight, and the crowd is growing more and more.\nUnfortunately, he picks you from the crowd and drags you in the arena. You will have to fight."},
    {15, "You are disgusted at the sight of an illigal animal fight club, where people bet on animals trained to kill eachother.\nSome people noticed that you are not supposed to be here.\nWill you have the guts to beat them up, though?"},
};

/*  =========================
          Save files
    ========================= */

int stageOfSave(string index) {
    fstream f("savefile" + index + ".txt", ios::in);
    string line;
    if (f.is_open()) {
        getline(f, line);
        f.close();
        return stoi(line);
    }
    else
        return 0;
}

bool loadGame(string index) {
    fstream f("savefile" + index + ".txt", ios::in);
    vector<string> lines;
    string line;
    if (f.is_open()) {
        getline(f, line);
        while (f) {
            lines.push_back(line);
            getline(f, line);
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

map<int, int> stageSaves{
    {0, 1},
    {1, 1},
    {7, 1},
    {8, 1},
    {10, 1},
};

/*  =========================
            Main
    ========================= */

string playStage(bool dodialogue = true) {
    if (respawn) {
        respawn = false;
        stage = 0;
        coins = defCoins;
        sanity = defSanity;
        health = defHealth;
        stamina = defStamina;
        playerDmg = defPlayerDmg;
        playerRes= defPlayerRes;
    }
    if (dodialogue) {
        clear();
        typeOut(dialogue[stage]);
        if (coinrewards[stage] != 0) {
            typeOut("You gained " + colored(to_string(coinrewards[stage]) + " coins", "text", "yellow") + "!");
            coins += coinrewards[stage];
            coinrewards[stage] = 0;
        }
        if (getItem[stage] != 0) {
            if (addToInventory(getItem[stage]) == false)
                typeOut("You wanted to pick up an item, but your inventory was full! \nCome back another time.");
            else {
                typeOut("You aquired " + colored(items[getItem[stage]].name, "text", "magenta") + "!");
                getItem[stage] = 0;
            }
        }
        typeOut("Coins:\t\t" + colored(to_string(coins), "text", "yellow"));
        typeOut("Stamina:\t" + colored(to_string(stamina), "text", "blue"));
        typeOut("Sanity:\t\t" + colored(to_string(sanity), "text", "red"));
        typeOut("Inventory:\t" + colored(to_string(inventory.size()) + "/" + to_string(inventorymax), "text", "magenta"));
    }
    map<string, string> options;
    map<string, string> specialOptions;
    options.insert(pair<string, string>("1n", "Go north"));
    options.insert(pair<string, string>("2e", "Go east"));
    options.insert(pair<string, string>("3s", "Go south"));
    options.insert(pair<string, string>("4w", "Go west"));
    options.insert(pair<string, string>("5m", "Consult your map"));
    options.insert(pair<string, string>("6i", "View your inventory"));
    options.insert(pair<string, string>("7r", "Rest"));
    if (shopList[stage] != 0)
        specialOptions.insert(pair<string, string>("1h", "Interact with the shop"));
    if (minigames[stage] != 0)
        specialOptions.insert(pair<string, string>("2g", minigametext[minigames[stage]]));
    if (stageSaves[stage] != 0)
        specialOptions.insert(pair<string, string>("3f", "Save game"));
    return optionsNav(options, specialOptions, "", [](int i) { return (directions[stage][i] >= 0); }, [](int i) { return (checkedMap && i < 4); });
}

void moveDirection(int direction, string name) {
    if (stamina == 0) { 
        typeOut("Your " + colored("stamina", "text", "blue") + " is depleted to 0. You have to rest first. [Press Enter]");
        getCin();
    }
    else {
        stamina -= 1;
        int state = directions[stage][direction];
        if (state < 0) { // dead end
            state = -1 - state; // get state to be the right index for deadends
            typeOut(deadend[state]);
            if (sanity > 0) {
                sanity -= 1;
                typeOut("Your " + colored("sanity", "text", "red") + " was depleted by 1.");
            }
            else
                typeOut("You are already insane.");
            typeOut("Your " + colored("stamina", "text", "blue") + " was depleted by 1 and you ended up back at the same place.");
            typeOut("You should have consulted your map. [Press Enter]");
            getCin();
        }
        else if (needItem[state] != 0 && !inventoryHas(needItem[state])) {
            typeOut("You will need a " + colored(items[needItem[state]].name, "text", "magenta") + " to pass through here. [Press Enter]");
            getCin();
        }
        else if (coins < needcoins[state]) {
            typeOut("You will need " + colored(to_string(needcoins[state]), "text", "yellow") + " coins to pass through here. [Press Enter]");
            getCin();
        }
        else {
            needItem[state] = 0;
            if (needcoins[state] != 0) { coins -= needcoins[state]; }
            needcoins[state] = 0;
            cout << "Moving " + name;
            cout << "." << flush;
            MSDelay(1000);
            cout << "." << flush;
            MSDelay(1000);
            cout << "." << flush;
            MSDelay(1000);
            typeOut(" [Press Enter]");
            getCin();
            bool defeated = false;
            checkedMap = false;
            prevstage = stage;
            stage = state;
            auto it = stageEncounters.find(state);
            if (it != stageEncounters.end() && it->second != 0) {
                clear();
                typeOut(stageDialogue[state] + " [Press Enter]");
                getCin();
                defeated = battle(it->second - 1);
                if (defeated)
                    it->second = 0;
            }
        }
    }
}

bool processInput(string input) {
    if (input == "m") { 
        checkedMap = true;
        typeOut("\n" + mapToString(), 0);
        return false;
    }
    else if (input == "i") {
        interactInventory();
        return false;
    }
    else if (input == "r") {
        rest();
        return false;
    }
    else if (input == "h" && shopList[stage] != 0) {
        openShop();
        return false;
    }
    else if (input == "g" && minigames[stage] != 0) {
        doMinigame();
        minigames[stage] = 0;
        return true;
    }
    else if (input == "f" && stageSaves[stage] != 0) {
        typeOut("Saving your game...");
        saveGame(currentFileIndex);
        MSDelay(2000);
        typeOut("Game Saved! [Press Enter]");
        getCin();
        cout << endl;
        return false;
    }
    else if (input == "n") { // North = 0
        moveDirection(0, "North");
        return true;
    }
    else if (input == "e") { // East = 1
        moveDirection(1, "East");
        return true;
    }
    else if (input == "s") { // South = 2
        moveDirection(2, "South");
        return true;
    }
    else if (input == "w") { // West = 3
        moveDirection(3, "West");
        return true;
    }
    else {
        typeOut("I am not sure what you meant with '" + input + "', but its not a valid option.");
        return false;
    }
}

void music() {
    return; // Music too big on github. 
}

int main()
{
    loadAssets();
    io.init();
    cin.tie(0);
    ios::sync_with_stdio(false);
    for (int i = 1; i < 4; i++) {
        fstream f("savefile" + to_string(i) + ".txt", ios::in);
        if (!f.is_open())
            saveGame(to_string(i)); // create default save files in non existant
        f.close();
    }
    typeOut("Please select a save file:");
    map<string, string> options;
    map<string, string> specialOptions;
    specialOptions.insert(pair<string, string>("1", "Save file 1 at " + stages[stageOfSave("1")]));
    specialOptions.insert(pair<string, string>("2", "Save file 2 at " + stages[stageOfSave("2")]));
    specialOptions.insert(pair<string, string>("3", "Save file 3 at " + stages[stageOfSave("3")]));
    string input = optionsNav(options, specialOptions, "Save");
    typeOut("Loading save file " + input + "...");
    loadGame(input);
    currentFileIndex = input;
    MSDelay(1000);
    bool dodialogue = true;
    while(true) {
        if (dodialogue) { possibleEncounter(); }
        input = playStage(dodialogue);
        dodialogue = processInput(input);
    }
    io.uninit();
}

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
#include "headers/system_io/input.h"
#include "headers/system_io/output.h"
#include "headers/file_system/assets.h"
#include "headers/file_system/saving.h"
#include "headers/game_mechanics/inventory.h"
#include "headers/game_mechanics/items.h"
#include "headers/game_mechanics/battle.h"
#include "headers/game_mechanics/shop.h"
#include "headers/misc/io_utils.h"
#include "headers/misc/utilities.h"
using namespace std;

MyIO io;

bool checkedMap = false;
int moral = 0;
bool respawn = false;
string currentFileIndex = "1";
saveState cursave;
int prevstage = cursave.stage;

// Stages
vector<string> stages = {
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

vector<StyleString> deadend = {
    (StyleString)"Walking here you can't see anything due to the thick fog. Anxious that you might trip and fall down a cliff, you return.",
    (StyleString)"Walking here makes you feel uncomfortable, as it's a very steep way down. You decide it's too risky.",
    (StyleString)"Sometimes it's better to not be greedy and take what you have, who knows what dangers lie beyond.",
    (StyleString)"You were about to step forward when you heard a howl. You decide it's not a good idea to head this way.",
    (StyleString)"Are you sure you want to head off path? It doesn't look like a good idea.",
    (StyleString)"You are not so sure if it's a good idea to head off path... where should you go?",
    (StyleString)"You head that way and bump yourself into a wall... maybe you should have thought twice.",
    (StyleString)"You look that way and see a solid gate... what were you thinking heading that way?",
    (StyleString)"You see a dark void further in the mine and state its dangerously steep, so you decide it's better to not go that way.",
};

vector<StyleString> dialogue = {
    (StyleString)"You find yourself seated on top of a rock, looking around you you can only see thick fog. \nYou are armed with a map and compass, and some " + colored("coins", Color::Yellow)+ ".",
    (StyleString)"You feel a fresh but cold wind breezing over your neck, and around you you see only white. \nYou read carven on a stone that you can press the left arrow key to make me talk faster.\nThe old man left a note: 'You will encounter enemies randomly, defeat them for coins!'\nYou spot a bird in the distance.",
    (StyleString)"You climbed this beast of a mountain, enourmous. \nBut you also knew, that you are so high up here that one mistep could be catastrophic.",
    (StyleString)"You travel along a silent river stream. \nYou suspect it gets its water from smelting ice up another mountain.",
    (StyleString)"You encounter a stone path, that probably leads to something promising, a form of life or civilization maybe?",
    (StyleString)"You approach something shiny, wow! Its a treasure chest! \nYou are very lucky that you found something this helpful this early. \nYou grab what you need and are sattisfied your efforts paid off.",
    (StyleString)"You see a cracky abandoned house and enter it. \nYou try to scout for loot, but can only find a dusty key.",
    (StyleString)"You see in front of you an enourmous gate. \nIt says that inside you can find a village. But you need to pay the guard a fee first.\nHe approaches you and explains that you need " + colored("20", Color::Yellow) + " coins to enter.",
    (StyleString)"You find yourself in the main village hub. \nYou see a lovely tree in the middle, and there are lots of ways to go.",
    (StyleString)"You enter building which claims it to be a shop.\nIndeed, when you enter, you are greeted to a what seems to be a friendly man.",
    (StyleString)"You fended off the duelist. The surrounding villagers are very pleased about your act. \nThere will be peace here, for now...",
    (StyleString)"You enter a creepy alley. It's dark and looks abadoned.\nIf this would be the only part of the village you'd be allowed to see, you'd think it's empty and dead.",
    (StyleString)"You see two artists acting on the street. \nYou stand and watch, wondering if they will be performing here for eternity...",
    (StyleString)"You stand before a solid, big wall. You read, carved on the stone: \n'Here stands, the West Wall!'\nYou notice the first half of the wall is made of smooth stone, while the other half is made of cobblestone.\nYou wonder if the second half was built for precaution, like someone tried to climb over it in the past...",
    (StyleString)"You enter a mine, with the indistinguishable smell of coal.\nIt's less aromatic than wood smoke and has a dirtier smell, like cigar smoke compared to a pipe mixture.",
    (StyleString)"You enter a local pub, where old and young men sit down and have a laugh together while drinking.\nYou see a door in the corner, and a man in a black jacket entering it.",
    (StyleString)"",
};

/*  =========================
            Directions
    ========================= */

vector<array<int, 4>> directions = {
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

map<int, int> needItem {
    {5, 1},         // for treasure chest, need key
    {15, 2},        // to enter club, need premium pass           
};

map<int, int> getItem {
    {6, 1}      // Get key in house
};

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

map<int,int> shops = {
    {9, 0},
};
map<int,Shop> shopList = {
    {0, {
            {{2, 20, 1}, {3, 5, 5}, {4, 5, 2}, {5, 2, 5}},
            {{1, 1}, {2, 10}, {3, 2}, {4, 2}, {5, 1}}
        }} // on stage 9 shop #1
};

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
        os << PIXEL_RESET << endl;
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
    if (minigames[cursave.stage] == 1) {
        typeOut(io, "A local miner offers you to strike a boulder to maybe find gold!");
        cout << minigameToString(0) << endl;
        typeOut(io, "Pick one boulder to mine.");
        std::vector<Option> options;
        options.push_back({"First Boulder", 1, Color::Blue});
        options.push_back({"Second Boulder", 2, Color::Blue});
        options.push_back({"Third Boulder", 3, Color::Blue});
        int choice = optionsNav(io, options, "Mine");
        if (randomnum(3) == 0) {
            cout << minigameToString(choice+3);
            cursave.coins += 20;
            if (choice == 1)
                typeOut(io, "You strike the first boulder and find some weird, valuable ore!");
            if (choice == 2)
                typeOut(io, "You strike the second boulder and find some weird, valuable ore!");
            if (choice == 3)
                typeOut(io, "You strike the third boulder and find some weird, valuable ore!");
            typeOut(io, "You sell it and gain " + colored("20", Color::Yellow) + " coins. [Press Enter]");
        }
        else {
            cout << minigameToString(choice);
            if (choice == 1)
                typeOut(io, "You strike the first boulder and find nothing, unfortunately. [Press Enter]");
            if (choice == 2)
                typeOut(io, "You strike the second boulder and find a worm, but nothing of value. [Press Enter]");
            if (choice == 3)
                typeOut(io, "You strike the third boulder and get yourself nothing but painful arms. [Press Enter]");
        }
        wait_enter(io); 
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

vector<image<16>> mapvis = {
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

vector<image<6>> maplegend = {
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou",string(PIXEL_BLUE) + string(PIXEL_RESET) + "\tWater"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou","T\tTreasure"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou","G\tGuard"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou","S\tShopkeeper"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou","C\tCrowd"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou","P\tPerformer"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou"},
    {string(PIXEL_GREEN) + string(PIXEL_RESET) + "\tGrass",string(PIXEL_WHITE) + string(PIXEL_RESET) + "\tRock",string(PIXEL_YELLOW) + string(PIXEL_RESET) + "\tPath",string(PIXEL_CYAN) + string(PIXEL_RESET) + "\tYou", string(PIXEL_RED) + string(PIXEL_RESET) + "\tMineral Ore","W\tWorkers"},
};

vector<StyleString> mapconclusion = {
    (StyleString)"It looks like North is the only reasonable option.",
    (StyleString)"You can't see any dead ends.",
    (StyleString)"Going either North or South looks sketchy.",
    (StyleString)"You can't decide which is worse, east or south.",
    (StyleString)"North seems to lead to something promising, south leads you back.",
    (StyleString)"You should head back.",
    (StyleString)"You conclude you should go back.",
    (StyleString)"You need to head through, so you need " + colored("20", Color::Yellow) + " coins.",
    (StyleString)"The nice center of the village, the main hub.\nYou see to the north a big road.",
    (StyleString)"If you want to leave, you should head out again, right?",
    (StyleString)"There are plenty of ways to go.",
    (StyleString)"You can go north, south, or west.",
    (StyleString)"You should conclude: No dead ends!",
    (StyleString)"You can go every way except west, obviously.",
    (StyleString)"A local worker warns you about going deeper into the mine.",
};

string mapToString() {
    ostringstream os;
    for (int i = 0; i < (int)maplegend[cursave.stage].size(); i++) {
        compass[i + 2] = " " + maplegend[cursave.stage][i];
    }
    for (int i = 0; i < (int)mapvis[cursave.stage].size(); i++) {
        for (int j = 0; j < (int)mapvis[cursave.stage][i].length(); j++) {
            if (mapvis[cursave.stage][i][j] == '0')
                os << PIXEL_GREEN;
            else if (mapvis[cursave.stage][i][j] == '1')
                os << PIXEL_WHITE;
            else if (mapvis[cursave.stage][i][j] == '2')
                os << PIXEL_YELLOW;
            else if (mapvis[cursave.stage][i][j] == 'Y')
                os << PIXEL_CYAN;
            else if (mapvis[cursave.stage][i][j] == '3')
                os << PIXEL_BLUE;
            else if (mapvis[cursave.stage][i][j] == '4')
                os << PIXEL_RED;
            else 
                os << colored(toString(mapvis[cursave.stage][i][j]), Color::Yellow, Color::Yellow).to_string();
        }
        os << PIXEL_RESET;
        if (compass[i] != "")
            os << compass[i];
        os << "\n";
    }
    os << "\n" << mapconclusion[cursave.stage].to_string() << "\n";
    return os.str();
}

/*  =========================
              Rest
    ========================= */

void rest() {
    typeOut(io, "\nYou decided to lie down and look up to the clear sky and rest for a bit.\nYou rest your eyes.\nResting...");
    set_cursor(false);
    double progress = 0.0;
    while (progress <= 1.0) {
        double barWidth = 70;
        double pos = barWidth * progress;
        for (int i = 0; i < (int)barWidth; i++) {
            if (i <= pos) std::cout << PIXEL_RED;
            else std::cout << PIXEL_WHITE;
        }
        cout << PIXEL_RESET << " " << int(progress * 100.0) << " %\r";
        cout.flush();

        progress += 0.01;
        MSDelay(100);
    }
    for (int i = 0; i < 70; i++) {
        cout << PIXEL_RED;
    }
    cout << PIXEL_RESET << " 100%\n";
    set_cursor(true);
    cursave.stamina = defStamina;
    if (cursave.sanity > 7) {
        cursave.sanity = 10;
        typeOut(io, "You are done resting! You feel a lot better.\nYour stamina reset back to " + colored(to_string(defStamina), Color::Blue) + ".\nYour sanity was set to " + colored("10", Color::Red) + ".");
    }
    else {
        cursave.sanity += 3;
        typeOut(io, "You are done resting! You feel a lot better.\nYour stamina reset back to " + colored(to_string(defStamina), Color::Blue) + ".\nYour sanity increased by " + colored("3", Color::Red) + ".");
    }
}

/*  =========================
         Battle System
    ========================= */

vector<vector<int>> possibleEncounters = {
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
    if (possibleEncounters[cursave.stage][possibleEncounters[cursave.stage].size() - 1] != 0)
        if (randomnum(possibleEncounters[cursave.stage][possibleEncounters[cursave.stage].size() - 1]) == 0 )
            battle(possibleEncounters[cursave.stage][randomnum(possibleEncounters[cursave.stage].size()-1)], cursave, respawn, io, prevstage);
}

map<int, int> stageEncounters { // always add 1 to enemy number, tutorial = 0 but we type in 1.
    {1, 1},             // In stage 1 ; Tutorial     
    {10, 5},            // In stage 10 ; duelist
    {15, 13},           // In stage 15 ; Angry man
};

map<int, StyleString> stageDialogue{
    {1, (StyleString)"On your way you encounter an old man who wants to help you on your journey. He says:\n'You will face enemies in your way! Let me train you.'\nYou accept. He states:\n'Use me as a training dummy. \nRemember: The fight only ends when I am " + colored("defeated", Color::Green) + ", you are defeated or if you " + colored("retreat", Color::Red) + ".\nYou can " + colored("attack", Color::Green) + " me by choosing the attack option then to strike as much to the middle as possible.\nAfter your turn I will attack, try to dodge my " + colored("spikes", Color::Red) + "!\nUse your arrow keys to " + colored("dodge", Color::Green) + ".'"},
    {10, (StyleString)"You spot a what seems to be angry duelist, because his opponent didn't show up to the fight.\nYou can tell he is hungry for a fight, and the crowd is growing more and more.\nUnfortunately, he picks you from the crowd and drags you in the arena. You will have to fight."},
    {15, (StyleString)"You are disgusted at the sight of an illigal animal fight club, where people bet on animals trained to kill eachother.\nSome people noticed that you are not supposed to be here.\nWill you have the guts to beat them up, though?"},
};

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

int playStage(bool dodialogue = true) {
    if (respawn) {
        respawn = false;
        cursave.reset();
    }
    if (dodialogue) {
        clear();
        typeOut(io, dialogue[cursave.stage]);
        if (coinrewards[cursave.stage] != 0) {
            typeOut(io, "You gained " + colored(to_string(coinrewards[cursave.stage]) + " coins", Color::Yellow) + "!");
            cursave.coins += coinrewards[cursave.stage];
            coinrewards[cursave.stage] = 0;
        }
        if (getItem[cursave.stage] != 0) {
            if (cursave.inventory.addToInventory(getItem[cursave.stage]) == false)
                typeOut(io, "You wanted to pick up an item, but your inventory was full! \nCome back another time.");
            else {
                typeOut(io, "You aquired " + colored(items[getItem[cursave.stage]].name, Color::Magenta) + "!");
                getItem[cursave.stage] = 0;
            }
        }
        typeOut(io, "Coins:\t\t" + colored(to_string(cursave.coins), Color::Yellow));
        typeOut(io, "Stamina:\t" + colored(to_string(cursave.stamina), Color::Blue));
        typeOut(io, "Sanity:\t\t" + colored(to_string(cursave.sanity), Color::Red));
        typeOut(io, "Inventory:\t" + colored(to_string(cursave.inventory.size()) + "/" + to_string(cursave.inventory.max_elements), Color::Magenta));
    }
    // map<string, string> options;
    // map<string, string> specialOptions;
    std::vector<Option> options;
    std::vector<std::string> dire_text = {
        "Go north",
        "Go east",
        "Go south",
        "Go west"
    };
    for (int i=0;i<4;i++){
        options.push_back({dire_text[i], i, checkedMap?directions[cursave.stage][i] >= 0?Color::Green:Color::Red:Color::White});
    }
    options.push_back({"Consult your map", 4});
    options.push_back({"View your inventory", 5});
    options.push_back({"Rest", 6});
    if (shops[cursave.stage] != 0){
        options.push_back({"Interact with the shop", -1, Color::Blue});
    }
    if (minigames[cursave.stage] != 0){
        options.push_back({minigametext[minigames[cursave.stage]], -2, Color::Blue});
    }
    if (stageSaves[cursave.stage] != 0){
        options.push_back({"Save game", -3, Color::Blue});
    }
    cout << "\n";
    return optionsNav(io, options, "");
}

void moveDirection(int direction, string name) {
    if (cursave.stamina == 0) { 
        typeOut(io, "Your " + colored("stamina", Color::Blue) + " is depleted to 0. You have to rest first. [Press Enter]");
        wait_enter(io);
    }
    else {
        cursave.stamina -= 1;
        int state = directions[cursave.stage][direction];
        if (state < 0) { // dead end
            state = -1 - state; // get state to be the right index for deadends
            typeOut(io, deadend[state]);
            if (cursave.sanity > 0) {
                cursave.sanity -= 1;
                typeOut(io, "Your " + colored("sanity", Color::Red) + " was depleted by 1.");
            }
            else
                typeOut(io, "You are already insane.");
            typeOut(io, "Your " + colored("stamina", Color::Blue) + " was depleted by 1 and you ended up back at the same place.");
            typeOut(io, "You should have consulted your map. [Press Enter]");
            wait_enter(io);
        }
        else if (needItem[state] != 0 && !cursave.inventory.inventoryHas(needItem[state])) {
            typeOut(io, "You will need a " + colored(items[needItem[state]].name, Color::Magenta) + " to pass through here. [Press Enter]");
            wait_enter(io);
        
        }
        else if (cursave.coins < needcoins[state]) {
            typeOut(io, "You will need " + colored(to_string(needcoins[state]), Color::Yellow) + " coins to pass through here. [Press Enter]");
            wait_enter(io);
        }
        else {
            needItem[state] = 0;
            if (needcoins[state] != 0) { cursave.coins -= needcoins[state]; }
            needcoins[state] = 0;
            cout << "Moving " + name;
            cout << "." << flush;
            MSDelay(1000);
            cout << "." << flush;
            MSDelay(1000);
            cout << "." << flush;
            MSDelay(1000);
            typeOut(io, " [Press Enter]");
            wait_enter(io);    
            bool defeated = false;
            checkedMap = false;
            prevstage = cursave.stage;
            cursave.stage = state;
            auto it = cursave.stageEncounters.find(state);
            if (it != cursave.stageEncounters.end() && it->second != 0) {
                clear();
                typeOut(io, stageDialogue[state] + " [Press Enter]");
                wait_enter(io);
                defeated = battle(it->second - 1, cursave, respawn, io, prevstage);
                if (defeated)
                    it->second = 0;
            }
        }
    }
}

bool processInput(int input) {
    if (input == 4) { // consult map
        checkedMap = true;
        typeOutLine(io, "\n" + mapToString());
        return false;
    }
    else if (input == 5) { // view inventory
        cursave.inventory.interactInventory(cursave, io);
        return false;
    }
    else if (input == 6) { // rest
        rest();
        return false;
    }
    else if (input == -1 && shops[cursave.stage] != 0) {
        shop(io, cursave, shopList[shops[cursave.stage]]);
        return false;
    }
    else if (input == -2 && minigames[cursave.stage] != 0) {
        doMinigame();
        minigames[cursave.stage] = 0;
        return true;
    }
    else if (input == -3 && stageSaves[cursave.stage] != 0) {
        typeOut(io, "Saving your game...");
        saveGame(currentFileIndex, cursave, cursave.inventory.max_elements);
        MSDelay(2000);
        typeOut(io, "Game Saved! [Press Enter]");
        wait_enter(io);
        cout << endl;
        return false;
    }
    else if (input == 0) { // North = 0
        moveDirection(0, "North");
        return true;
    }
    else if (input == 1) { // East = 1
        moveDirection(1, "East");
        return true;
    }
    else if (input == 2) { // South = 2
        moveDirection(2, "South");
        return true;
    }
    else if (input == 3) { // West = 3
        moveDirection(3, "West");
        return true;
    }
    else {
        typeOut(io, "I am not sure what you meant with '" + to_string(input) + "', but its not a valid option.");
        return false;
    }
}

void music() {
    return; // Music too big on github. 
}

// used for tranferring stanges into file, not necessary for game
Stage makeStage(int i){
    Stage out;
    out.id = i;
    out.name = stages[i];
    out.dialogue = dialogue[i];
    out.conclusion = mapconclusion[i];
    out.directions = directions[i];
    out.vis = mapvis[i];
    out.legend = maplegend[i];
    out.needcoins = needcoins[i];
    out.coinreward = coinrewards[i];
    if(shops.count(i))
        out.shop = shops[i];
    if(minigames.count(i))
        out.minigame = minigames[i];
    if(stageEncounters.count(i))
        out.encounter = std::make_pair(stageEncounters[i], stageDialogue[i]);
    
        
    if(possibleEncounters[i][3] != 0)
    for(int j = 0; j < 3; j++)
    {
        out.possibleEncounters.emplace_back(possibleEncounters[i][j], 1.0 / (double)possibleEncounters[i][3]);
    }
    return out;
}

int main()
{
    try {
        loadAssets();
        io.init();
        battle(13, cursave, respawn, io, prevstage);
        // File saving
        for (int i = 1; i < 4; i++) {
            fstream f("saves/savefile" + to_string(i) + ".txt", ios::in);
            if (!f.is_open()) {
                saveState s;
                saveGame(to_string(i), s, cursave.inventory.max_elements); // create default save files in non existant
            }
            f.close();
        }
        typeOut(io, "Please select a save file:");
        std::vector<std::string> files = {
            "1",
            "2",
            "3"
        };
        std::vector<Option> options;
        for (int i=0;i<files.size();i++){
            options.push_back({"Save file " + files[i] + " at " + stages[stageOfSave(files[i])], i, Color::Blue});
        }
        int choice = optionsNav(io, options, "Save");
        std::string file = files[choice];
        typeOut(io, "Loading save file " + file + "...");
        auto [suc, ret] = loadGame(file);
        cursave = ret;
        currentFileIndex = file;
        MSDelay(1000);
        bool dodialogue = true;
        while(true) {
            if (dodialogue) { possibleEncounter(); }
            int input = playStage(dodialogue);
            dodialogue = processInput(input);
        }
    } catch(const Interupt &e){
        printf("\nGame Quit\n");
    }
    set_cursor(true);
    io.uninit();
}

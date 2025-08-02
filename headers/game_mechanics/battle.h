#pragma once
#include "../system_io/input.h"
#include "../system_io/output.h"
#include "../file_system/assets.h"
#include "../file_system/saving.h"
#include "../misc/io_utils.h"
#include "../misc/utilities.h"
using namespace std;

// const double FPS = 2.0; // Default: 15.0
const double FPS = 20.0; // Default: 15.0
const int X_SIZE = 101;
const int Y_SIZE = 11;

const vector<vector<int>> battleVis = {
    {160,160,160,160,166,166,166,166,166,172,172,172,172,178,178,178,178,184,184,184,184,184,82,46,46,35,31,68,56,21,93,165,201,165,93,21,56,68,31,35,46,46,82,184,184,184,184,184,178,178,178,178,172,172,172,172,166,166,166,166,166,160,160,160,160},
    {160,160,160,160,166,166,166,166,166,172,172,172,172,178,178,178,178,184,184,184,184,184,82,46,46,35,31,68,56,21,93,165,201,165,93,21,56,68,31,35,46,46,82,184,184,184,184,184,178,178,178,178,172,172,172,172,166,166,166,166,166,160,160,160,160},
    {160,160,160,160,166,166,166,166,166,172,172,172,172,178,178,178,178,184,184,184,184,184,82,46, 46,35,31,68,56,21,93,165,201,165,93,21,56,68,31,35,46,46,82,184,184,184,184,184,178,178,178,178,172,172,172,172,166,166,166,166,166,160,160,160,160},
};

string opponentToString(int opponentnmr) {
    ostringstream os;
    for (int i = 0; i < (int)(opponents[opponentnmr].vis).size(); i++) {
        for (int j = 0; j < (int)opponents[opponentnmr].vis[i].length(); j++) {
            if (opponents[opponentnmr].vis[i][j] == '0')
                os << PIXEL_GREEN;
            else if (opponents[opponentnmr].vis[i][j] == '1')
                os << PIXEL_WHITE;
            else if (opponents[opponentnmr].vis[i][j] == '2')
                os << PIXEL_YELLOW;
            else if (opponents[opponentnmr].vis[i][j] == '3')
                os << PIXEL_BLUE;
            else if (opponents[opponentnmr].vis[i][j] == '4')
                os << PIXEL_RED;
            else if (opponents[opponentnmr].vis[i][j] == '5')
                os << PIXEL_MAGENTA;
            else if (opponents[opponentnmr].vis[i][j] == '6')
                os << PIXEL_BLACK;
            else if (opponents[opponentnmr].vis[i][j] == '7')
                os << PIXEL_CYAN;
        }
        os << PIXEL_RESET << "\n";
    }
    return os.str();
}

bool battle(int opponentnmr, saveState& cursave, bool& respawn, MyIO &io, int& prevstage) {
    bool retreat = false;
    bool giveOptions = true;
    int damage = 0;
    int speed = 0;
    bool attack = true;
    cout << opponentToString(opponentnmr) << "\n";
    typeOut(io, "You encounter an enemy! [Press Enter]");
    wait_enter(io);
    int opponenthealth = opponents[opponentnmr].hp;
    while (!retreat && opponenthealth > 0 && cursave.health > 0) {
        attack = true;
        if (giveOptions)
            clear();
        if (damage != 0) { damage -= 1; typeOut(io, colored("Extra Damage", Color::Red)); }
        if (speed != 0) { speed -= 1; typeOut(io, colored("Extra Speed", Color::Cyan)); }
        typeOut(io, "Your Health: " + colored(to_string(cursave.health), Color::Red) + "\nEnemy Health: " + colored(to_string(opponenthealth), Color::Red));
        std::vector<Option> options;
        options.push_back({"Attack Enemy", -1, Color::Blue});
        options.push_back({"Use A Potion", -2, Color::Blue});
        options.push_back({"Retreat", -3, Color::Blue});
        int choice = optionsNav(io, options, "Fight");
         if (choice == -1) {
            // attack
            double dmgmul = 1.0;
            if (damage != 0)
                dmgmul = 1.8;
            typeOut(io, "You prepare an attack! Press the up arrow key to strike!");
            set_cursor(false);
            ostringstream os;
            for (size_t i = 0; i < battleVis.size(); i++) {
                os << "\n ";
                for (int j = 0; j < (int)battleVis[i].size(); j++) {
                    os << "\033[48;5;" << to_string(battleVis[i][j]) << "m ";
                }
                os << PIXEL_RESET;
            }
            cout << os.str() << "\n";
            bool forward = true;
            double progress = 0.0;
            do {
                io.check();
                std::cout << "[";
                double pos = battleVis[0].size() * progress;
                for (int i = 0; i < (int)battleVis[0].size(); i++) {
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
            int dmg = (int)(cursave.playerDmg* (1-abs(1-2* progress)) * dmgmul + 4);
            dmg -= (int)(dmg * ((double)opponents[opponentnmr].resistance / 100));
            if (dmg < 0) { dmg = 0; }
            if (opponenthealth - dmg < 0) { opponenthealth = 0; }
            else { opponenthealth -= dmg; }
            set_cursor(true);
            typeOut(io, "\nYou struck the enemy for " + colored(to_string(dmg), Color::Red) + " damage! It's now on " + colored(to_string(opponenthealth), Color::Red) + " health!");
            if (opponenthealth == 0) {
                attack = false;
                int coinreward = (randomnum(opponents[opponentnmr].coinrewardmax - opponents[opponentnmr].coinrewardmin + 1)) + opponents[opponentnmr].coinrewardmin;
                cursave.coins += coinreward;
                typeOut(io, "You defeated the enemy! You gained " + colored(to_string(coinreward), Color::Yellow) + " coins! [Press Enter]");
                wait_enter(io);
            }
            giveOptions = true;
        } else if (choice == -2) {
            typeOut(io, "What " + colored("potion", Color::Green) + " do you want to use?");
            string input2;
            std::vector<Option> options2;
            options2.push_back({"Health Potion", 0, Color::Magenta});
            options2.push_back({"Damage Potion", 1, Color::Red});
            options2.push_back({"Speed Potion", 2, Color::Cyan});
            options2.push_back({"Exit", -1, Color::Blue});
            cout << "Use up and down arrow keys to navigate, right arrow key to pick." << endl;
            int choice2 = optionsNav(io, options2, "Pot");
            if (choice2 == 0) {
                typeOut(io, "You went ahead and took a " + colored("Health Potion", Color::Magenta) + ".");
                if (cursave.health + 70 >= 100) {
                    cursave.health = 100;
                    typeOut(io, "Your " + colored("Health", Color::Magenta) + " was set to 100.");
                }
                else {
                    cursave.health += 70;
                    typeOut(io, "Your " + colored("Health", Color::Magenta) + " got increased by 70.");
                }
            } else if (choice2 == 1) {
                typeOut(io, "You went ahead and took a " + colored("Damage Potion", Color::Red) + ".");
                damage += 30;
                typeOut(io, "You gained the " + colored("Damage", Color::Red) + " effect for 3 rounds.");
            } else if (choice2 == 2) {
                typeOut(io, "You went ahead and took a " + colored("Speed Potion", Color::Cyan) + ".");
                speed += 2;
                typeOut(io, "You gained the " + colored("Speed", Color::Cyan) + " effect for 2 rounds.");
            }
            // if exit do nothing
        } else {
            retreat = true;
            attack = false;
            typeOut(io, "You retreat cowardly out of fear... you feel ashamed.");
            cursave.stage = prevstage;
            if (cursave.sanity - 2 > 0) {
                cursave.sanity -= 2;
                typeOut(io, "Your " + colored("sanity", Color::Red) + " was depleted by 2. [Press Enter]");
            }
            else {
                cursave.sanity = 0;
                respawn = true;
                typeOut(io, "Your " + colored("sanity", Color::Red) + " was drained to 0. You died of insanity. [Press Enter]");
            }
            wait_enter(io);
        }

        if (attack) {
            typeOut(io, "Remember to use the arrow Keys!");
            cout << "Enemy attack in 3... " << flush;
            MSDelay(1000);
            cout << "2... " << flush;
            MSDelay(1000);
            cout << "1... " << flush;
            MSDelay(1000);
            cout << "\r" << flush;
            set_cursor(false); // hide cursor
            int attackFrames = opponents[opponentnmr].attackdurance;
            // bool exitAttack = false;
            const int rows = Y_SIZE;
            const int charPerRow = X_SIZE;
            int playerX = 50;
            int playerY = 4;
            int speedmul = 1;
            if (speed != 0)
                speedmul = 2;
            const int invincibilityFramesMax = 10;
            const int invincibilityFrameChange = 2;
            int invincibilityFrames = 0;
            clock_t previousTime = clock();
            const double frameDelay = 1.0 / FPS;
            vector<vector<int>> attackVis(charPerRow+1, vector<int>(rows+1));

            const auto& playerHit = [&]() {
                if (attackVis[playerX][playerY] > 0 && attackFrames > 0) // hit?
                    if (invincibilityFrames == 0) { // player must not be invincible
                        invincibilityFrames = invincibilityFramesMax; // hit
                        int attackDamage = opponents[opponentnmr].attackdmg;
                        if (opponents[opponentnmr].dmgrange != 0) { // random damage range?
                            attackDamage += (randomnum(opponents[opponentnmr].dmgrange + 1)) - opponents[opponentnmr].dmgrange / 2; // pick number between 1 - damage range, subtract by damage range / 2
                        }
                        if (cursave.health - attackDamage < 0)
                            cursave.health = 0;
                        else
                            cursave.health -= attackDamage; // substract health
                    }
            };

            const auto& screenRefresh = [&]() {
                for (int i = 0; i < rows; i++) { // for every row
                    for (int j = 0; j < charPerRow; j++) { // for every char in row
                        if (i == playerY && j == playerX) {
                            // invincibility calculation
                            if (invincibilityFrames % invincibilityFrameChange > 0)
                                printf(PIXEL_RED); // player got hurt and is invincible
                            else
                                printf(PIXEL_CYAN);
                        }
                        else if (attackVis[j][i] > 0)
                            printf(PIXEL_WHITE);
                        else
                            printf(PIXEL_GREEN);
                    }
                    printf("%s\n", PIXEL_RESET);
                }
            };

            const auto& renderHealthbar = [&]() {
                cout << "\n";
                for (int i = 0; i < (int)charPerRow / 2 - 3; i++)
                    cout << " ";
                cout << "HEALTH" << "\n";
                if (cursave.health > 0) {
                    double healthBar = 100.0 / defHealth * cursave.health / 100.0;
                    double barWidth = charPerRow;
                    double pos1 = barWidth * healthBar;
                    for (int i = 0; i < (int)barWidth; i++) {
                        if (i <= pos1) cout << PIXEL_RED;
                        else std::cout << PIXEL_WHITE;
                    }
                    cout << PIXEL_RESET << " " << cursave.health << " %  " << "\n";
                }
                else {
                    for (int i = 0; i < 101; i++)
                        cout << PIXEL_WHITE;
                    cout << PIXEL_RESET << " 0 %    " << "\n";
                }
            };

            const auto& inputCheck = [&]() {
                io.check();
                if (io.pressed[K_UP] && playerY != 0 && attackFrames > 0)
                    playerY -= 1;
                if (io.pressed[K_DOWN] && playerY != rows - 1 && attackFrames > 0)
                    playerY += 1;
                if (io.pressed[K_LEFT] && playerX != 0 && attackFrames > 0)
                    playerX -= 1 * speedmul;
                if (io.pressed[K_RIGHT] && playerX != charPerRow - 1 && attackFrames > 0)
                    playerX += 1 * speedmul;
            };

            const auto& shared = [&]() {
                playerHit();
                screenRefresh();
                renderHealthbar();
                inputCheck();
            };

            const auto& sa = [&](int c, int r, int val=1)  {
                attackVis[max(min(c, charPerRow), 0)][max(min(r, rows), 0)] = val; 
            };

            if (opponents[opponentnmr].type == 0) { // is attack type spikes?
                // map<int, int> spikesX;
                vector<int> spikesX(charPerRow); 
                vector<int> spikesY(rows); 
                vector<int> spikesY2(rows); 
                vector<int> spikesY3(rows); 
                int chanceofspike = 5;
                int counteram = 5;
                if (opponents[opponentnmr].difficulty > 1) {
                    chanceofspike = 3;
                    counteram = 4;
                }
                if (opponents[opponentnmr].difficulty > 2) {
                    chanceofspike = 2;
                    counteram = 3;
                }
                if (opponents[opponentnmr].difficulty > 4) {
                    chanceofspike = 1;
                    counteram = 2;
                }
                // if (opponents[opponentnmr].difficulty > 2) // add 50% chance if difficulty is under or equal to 2
                //     int chanceofspikeX = 0;
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
                int counter = 0;
                while (attackFrames != 0 && cursave.health > 0) {
                    clock_t currentTime = clock();
                    double dt = double(currentTime - previousTime) / CLOCKS_PER_SEC;
                    if (dt < frameDelay) {
                        double timeLeft = frameDelay - dt;
                        MSDelay(timeLeft * 1000);
                    }
                    previousTime = currentTime;
                    counter++;
                    if (counter == counteram) {
                        counter = 0;
                        for (auto&i:attackVis) fill(i.begin(),i.end(),0);
                        if (invincibilityFrames > 0) // substract invincibility
                            invincibilityFrames -= 1;
                        if (spikes2Countdown > 0)
                            spikes2Countdown -= 1;
                        if (spikes3Countdown > 0)
                            spikes3Countdown -= 1;
                        attackFrames -= 1;
                        // anti AFK for levels above 1 ; if the delay is 0
                        if (afkSpikeDelayX == 0) {
                            afkSpikeDelayX = (randomnum(afkSpikeDelayMaxX));
                            if (opponents[opponentnmr].difficulty > 1) {
                                int point1 = playerX;
                                if (spikesX[point1] == 0) // is this spike already claimed?
                                    spikesX[point1] = rows * spikeMove + (randomnum(afkSpikeDelayMaxX)) * spikeMove; // Fall ; with custom delay
                            }
                        }
                        else
                            afkSpikeDelayX -= 1;
                        if (afkSpikeDelayY == 0) {
                            afkSpikeDelayY = (randomnum(afkSpikeDelayMaxY));
                            if (opponents[opponentnmr].difficulty > 1) {
                                int point2 = playerY;
                                if (spikesY[point2] == 0) // is this spike already claimed?
                                    spikesY[point2] = charPerRow * spikeMove; // no delay here, just immidiate fall
                            }
                        }
                        else
                            afkSpikeDelayY -= 1;
                        // spikes X
                        if (randomnum(chanceofspike) == 0) { // Is there going to be spike(s) summoned?
                            for (int i = 0; i < ammountSpikesX; i++) {
                                int point3 = randomnum(charPerRow); // Grab random spike pos
                                if (spikesX[point3] == 0) // is this spike already claimed?
                                    spikesX[point3] = rows * spikeMove + (randomnum(maxSpikeCooldown)) * spikeMove;
                            }
                        }
                        // spikes Y
                        if (opponents[opponentnmr].difficulty > 1) {
                            for (int i = 0; i < ammountSpikesY; i++) {
                                int point4 = randomnum(rows); // Grab random spike pos
                                if (spikesY[point4] == 0) // is this spike already claimed?
                                    spikesY[point4] = charPerRow * spikeMove;
                            }
                        }
                        if (opponents[opponentnmr].difficulty > 2 && spikes2Countdown == 0) {
                            for (int i = 0; i < ammountSpikesY; i++) {
                                int point5 = randomnum(rows); // Grab random spike pos
                                if (spikesY2[point5] == 0) // is this spike already claimed?
                                    spikesY2[point5] = charPerRow * spikeMove;
                            }
                        }
                        if (opponents[opponentnmr].difficulty > 3 && spikes3Countdown == 0) {
                            for (int i = 0; i < ammountSpikesY; i++) {
                                int point6 = randomnum(rows); // Grab random spike pos
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
                                    sa(x, 0);
                                else
                                    sa(x, rows - value);
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
                                        sa(0,x);
                                    else
                                        sa(charPerRow - value, x);
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
                                        sa(0, x);
                                    else
                                        sa(charPerRow - value, x);
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
                                        sa(0, x);
                                    else
                                        sa(charPerRow - value, x);
                                    spikesY[x] -= 1;
                                }
                            }
                        }
                    }
                    shared();
                    goBack(rows + 3);
                }
                for (int i = 0; i < rows + 3; i++) // for every row + healthbar
                    cout << "\n";
            }
            if (opponents[opponentnmr].type == 1) { // is attack type bombs?
                // index: column
                // first: positive: number of frame since creation. Move every bombSpeed frame
                //        negative: number of frame since explosion. Grow every explodeSpeed frame.
                // second: row to explode, 0 if no bomb on the column
                std::vector<std::pair<int,int>> bombs(charPerRow, {0,0});
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
                int chanceofbomb = 30;
                if (opponents[opponentnmr].difficulty > 2)
                    chanceofbomb = 50 - 10 * opponents[opponentnmr].difficulty;
                if (opponents[opponentnmr].difficulty > 5)
                    chanceofbomb = 10;
                // number of attack frame it takes for bomb to move
                int bombSpeed;
                if (opponents[opponentnmr].difficulty > 4){
                    bombSpeed = 6;
                } else if (opponents[opponentnmr].difficulty > 2){
                    bombSpeed = 8;
                } else {
                    bombSpeed = 12;
                }
                // number of attack frame it takes for shockwave to move
                int explodeSpeed;
                if (opponents[opponentnmr].difficulty > 4){
                    explodeSpeed = 3;
                } else if (opponents[opponentnmr].difficulty > 2) {
                    explodeSpeed = 4;
                } else {
                    explodeSpeed = 6;
                }
                // int maxBombCooldown = 10 * opponents[opponentnmr].difficulty;
                // if (opponents[opponentnmr].difficulty > 4) // bombs move faster
                //     maxBombCooldown = 40;
                bool upgradedBombs = false;
                if (opponents[opponentnmr].difficulty > 2)
                    upgradedBombs = true;
                int ammountbombs = 5;
                if (opponents[opponentnmr].difficulty > 2)
                    ammountbombs = 6;
                if (opponents[opponentnmr].difficulty > 4){
                    ammountbombs = opponents[opponentnmr].difficulty;
                }
                while (attackFrames > 0 && cursave.health > 0) {
                    clock_t currentTime = clock();
                    double dt = double(currentTime - previousTime) / CLOCKS_PER_SEC;
                    if (dt < frameDelay) {
                        double timeLeft = frameDelay - dt;
                        MSDelay(1000 * timeLeft);
                    }
                    previousTime = currentTime;
                    for (auto&i:attackVis) fill(i.begin(),i.end(),0);
                    if (invincibilityFrames > 0) // substract invincibility
                        invincibilityFrames -= 1;
                    attackFrames -= 1;
                    // bombs
                    for (int i = 0; i < ammountbombs; i++) {
                        if (randomnum(chanceofbomb) == 0) { // Is there going to be a bomb summoned?
                            int point = randomnum(charPerRow); // Grab random bomb pos
                            if (bombs[point].second == 0) {
                                bombs[point] = {
                                    0,
                                    randomnum(2, rows)
                                };
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
                    for (int col=0;col<charPerRow;col++){
                        pair<int, int> &bomb = bombs[col];
                        if (bomb.second == 0)continue;
                        if (bomb.first >= 0) {
                            int row = bomb.first / bombSpeed;
                            sa(col, row);
                            bomb.first++;
                            if (row == bomb.second){
                                bomb.first = -1;
                            }
                        } else {
                            int row = bomb.second;
                            int stage = (-bomb.first)/explodeSpeed;
                            bomb.first--;
                            if (stage == 0){
                                sa(col, row);
                            } else if (stage == 1) {
                                if (upgradedBombs) {
                                    sa(col - 1, row - 1);
                                    sa(col    , row - 1);
                                    sa(col + 1, row - 1);
                                    sa(col - 1, row    );
                                    sa(col + 1, row    );
                                    sa(col - 1, row + 1);
                                    sa(col    , row + 1);
                                    sa(col + 1, row + 1);
                                } else {
                                    sa(col    , row - 1);
                                    sa(col - 1, row    );
                                    sa(col + 1, row    );
                                    sa(col    , row + 1);
                                }
                            } else if (stage == 2) {
                                if (upgradedBombs) {
                                    sa(col - 1, row - 2);
                                    sa(col    , row - 2);
                                    sa(col + 1, row - 2);
                                    sa(col - 2, row - 1);
                                    sa(col - 1, row - 1);
                                    sa(col + 1, row - 1);
                                    sa(col + 2, row - 1);
                                    sa(col + 3, row    );
                                    sa(col + 2, row    );
                                    sa(col - 2, row    );
                                    sa(col - 3, row    );
                                    sa(col - 2, row + 1);
                                    sa(col - 1, row + 1);
                                    sa(col + 1, row + 1);
                                    sa(col + 2, row + 1);
                                    sa(col - 1, row + 2);
                                    sa(col    , row + 2);
                                    sa(col + 1, row + 2);
                                } else {
                                    sa(col - 1, row - 1);
                                    sa(col    , row - 1);
                                    sa(col + 1, row - 1);
                                    sa(col - 2, row    );
                                    sa(col + 2, row    );
                                    sa(col - 1, row + 1);
                                    sa(col    , row + 1);
                                    sa(col + 1, row + 1);
                                }
                            } else if (stage == 3) {
                                if (upgradedBombs) {
                                    sa(col - 2, row - 2);
                                    sa(col - 1, row - 2);
                                    sa(col    , row - 2);
                                    sa(col + 1, row - 2);
                                    sa(col + 2, row - 2);
                                    sa(col - 3, row - 1);
                                    sa(col - 2, row - 1);
                                    sa(col + 2, row - 1);
                                    sa(col + 3, row - 1);
                                    sa(col + 4, row    );
                                    sa(col + 3, row    );
                                    sa(col - 3, row    );
                                    sa(col - 4, row    );
                                    sa(col - 3, row + 1);
                                    sa(col - 2, row + 1);
                                    sa(col + 2, row + 1);
                                    sa(col + 3, row + 1);
                                    sa(col - 2, row + 2);
                                    sa(col - 1, row + 2);
                                    sa(col    , row + 2);
                                    sa(col + 1, row + 2);
                                    sa(col + 2, row + 2);
                                } else {
                                    sa(col - 1, row - 2);
                                    sa(col    , row - 2);
                                    sa(col + 1, row - 2);
                                    sa(col - 2, row - 1);
                                    sa(col + 2, row - 1);
                                    sa(col - 3, row    );
                                    sa(col + 3, row    );
                                    sa(col - 2, row + 1);
                                    sa(col + 2, row + 1);
                                    sa(col - 1, row + 2);
                                    sa(col    , row + 2);
                                    sa(col + 1, row + 2);
                                }
                            } else {
                                bomb.second = 0;
                            }
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
                                sa(x.first, rows - 1);
                                snake[x.first] -= 1;
                            }
                        }
                        if (snake2activate == 0) {
                            for (auto const& x : snake2) {
                                if (snake2[x.first] > 0) {
                                    sa(x.first, rows - 2);
                                    snake2[x.first] -= 1;
                                }
                            }
                        }
                    }
                    shared();
                    goBack(rows + 3);
                }
                for (int i = 0; i < rows + 3; i++) // for every row + healthbar
                    cout << "\n";
            }
            if (opponents[opponentnmr].type == 2) { // is attack type snakes?
                const int randomSnakeCountdown = 100;
                const int randomSnakeMoveMax = 3;
                int randomSnakeIgnorance = 3;
                if (opponents[opponentnmr].difficulty > 0)
                    randomSnakeIgnorance = 2;
                int positionsX[charPerRow]{};
                int snakeMoveX[charPerRow]{};
                int directionsX[charPerRow]{};
                for (int i = 0; i < charPerRow; i++) {
                    positionsX[i] = 0 - (randomnum(randomSnakeCountdown));
                    if (randomnum(randomSnakeIgnorance) == 0)
                        snakeMoveX[i] = 1 + (randomnum(randomSnakeMoveMax * 5));
                    else
                        snakeMoveX[i] = 0;
                    directionsX[i] = randomnum(2);
                }
                int positionsY[rows]{};
                int snakeMoveY[rows]{};
                int directionsY[rows]{};
                for (int i = 0; i < rows; i++) {
                    positionsY[i] = 0 - (randomnum(randomSnakeCountdown));
                    snakeMoveY[i] = 1 + (randomnum(randomSnakeMoveMax));
                    directionsY[i] = randomnum(2);
                }
                const int snakeduranceX = 1 * opponents[opponentnmr].difficulty;
                const int snakeduranceY = 5 * opponents[opponentnmr].difficulty;
                int counter = 0;
                while (attackFrames != 0 && cursave.health > 0) {
                    clock_t currentTime = clock();
                    double dt = double(currentTime - previousTime) / CLOCKS_PER_SEC;
                    if (dt < frameDelay) {
                        double timeLeft = frameDelay - dt;
                        MSDelay(1000*timeLeft);
                    }
                    previousTime = currentTime;
                    counter++;
                    if (counter == 3) {
                        counter = 0;
                        for (auto&i:attackVis) fill(i.begin(),i.end(),0);
                        if (invincibilityFrames > 0) // substract invincibility
                            invincibilityFrames -= 1;
                        attackFrames -= 1;
                        // update snake countdowns and summon new parts for X
                        for (int i = 0; i < charPerRow; i++) {
                            if (snakeMoveX[i] > 0) {
                                if (positionsX[i] < 0) {
                                    positionsX[i] += 1;
                                }
                                else {
                                    if (directionsX[i] == 0) {
                                        int value = positionsX[i] / snakeMoveX[i];
                                        sa(i, value, snakeduranceX * snakeMoveX[i]);
                                        if (positionsX[i] >= rows * snakeMoveX[i])
                                            positionsX[i] = 0;
                                        else
                                            positionsX[i] += 1;
                                    }
                                    else {
                                        int value = positionsX[i] / snakeMoveX[i];
                                        sa(i, value, snakeduranceX * snakeMoveX[i]);
                                        if (positionsX[i] <= 0)
                                            positionsX[i] = rows * snakeMoveX[i];
                                        else
                                            positionsX[i] -= 1;
                                    }
                                }
                            }
                        }
                        // update snake countdowns and summon new parts for Y
                        for (int i = 0; i < rows; i++) {
                            if (positionsY[i] < 0) {
                                positionsY[i] += 1;
                            } else {
                                if (directionsY[i] == 0) {
                                    int value = positionsY[i] / snakeMoveY[i];
                                    sa(value, i, snakeduranceY * snakeMoveY[i]);
                                    if (positionsY[i] >= charPerRow * snakeMoveY[i])
                                        positionsY[i] = 0;
                                    else
                                        positionsY[i] += 1;
                                }
                                else {
                                    int value = positionsY[i] / snakeMoveY[i];
                                    sa(value, i, snakeduranceY * snakeMoveY[i]);
                                    if (positionsY[i] <= 0)
                                        positionsY[i] = charPerRow * snakeMoveY[i];
                                    else
                                        positionsY[i] -= 1;
                                }
                            }
                        }
                    }
                    shared();
                    goBack(rows + 3);
                }
                for (int i = 0; i < rows + 3; i++) // for every row + healthbar
                    cout << endl;
            }
            set_cursor(true);
            if (cursave.health == 0) {
                typeOut(io, "You died in the fight, knowing you tried. [Press Enter]");
                respawn = true;
            }
            else
                typeOut(io, "The opponent finished attacking. [Press Enter]");
            wait_enter(io);
            giveOptions = true;
        }
    }
    if (opponenthealth > 0)
        return false;
    return true;
}
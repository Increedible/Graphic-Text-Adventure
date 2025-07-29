#include <iostream>
#include <vector>
#include <filesystem>
#include <array>

namespace fs = std::filesystem;

// an image of `n` rows of characters
template<int n>
using image=std::array<std::string, n>;

/// reads n lines from a `f` and writes them to `out`
/// @return 0 if succes, otherwise 1
template<int n>
int loadImage(FILE *f, image<n> &out) {
    if (f == NULL) {
        return 1;
    }

    char buffer[1024];  // Buffer to hold each line
    for(int i = 0; i < n; i++) {
        if(!fgets(buffer, sizeof(buffer), f))
            return 1;
        out[i] = buffer;
    }

    return 0;
}

template<int n>
void saveImage(FILE *f, image<n> &i)
{
    for(auto &l: i)
    {
        fprintf(f, "%s\n", l.c_str());
    }
}

/// list of images used by minigames
/// 0 ; Which ore?
/// 1 ; First boulder empty
/// 2 ; Second builder empty
/// 3 ; Third builder empty
/// 4 ; First boulder gold
/// 5 ; Second builder gold
/// 6 ; Third builder gold
std::vector<image<23>> minigameVis;

/// loads all images used by minigames into minigameVis
/// @return 0 if succes, otherwise 1
int loadMinigameAssets()
{
    const fs::path path = "assets/images/minigames";

    try {
        for(int i = 0;;i++) {
            fs::path p = path / fs::path(std::to_string(i) + ".tmg");
            if (!fs::exists(p)) {
                break;
            }
            FILE* file = fopen(p.string().c_str(), "r");
            minigameVis.emplace_back();
            loadImage<23>(file, minigameVis.back());
            fclose(file);
        }
    } catch (const fs::filesystem_error& e) {
        return 1;
    }

    return 0;
}

struct Enemy {
    int hp;
    int attackdmg;
    int attackdurance;
    int type; // 0 = Spikes, 1 = Melee, 2 = Ranged
    int difficulty; // 1-10
    int dmgrange; // 0 = no range, 10 = max range
    int resistance; // 0-100%
    int coinrewardmin;
    int coinrewardmax;
    image <23> vis;
};

std::vector<Enemy> opponents;

Enemy loadEnemy(FILE* f)
{
    Enemy out;
    fscanf(f, "%d %d %d %d %d %d %d %d %d\n", &out.hp, &out.attackdurance, &out.attackdmg, &out.type,
        &out.difficulty, &out.dmgrange, &out.resistance, &out.coinrewardmin, &out.coinrewardmax);
    
    loadImage<23>(f, out.vis);
    return out;
}

void saveEnemy(FILE* f, Enemy out)
{
    fprintf(f, "%d %d %d %d %d %d %d %d %d\n", out.hp, out.attackdmg, out.attackdurance, out.type,
        out.difficulty, out.dmgrange, out.resistance, out.coinrewardmin, out.coinrewardmax);
    saveImage<23>(f, out.vis);
}

int loadEnemyAssets()
{
    const fs::path path = "assets/enemies/";

    try {
        for(int i = 0;;i++) {
            fs::path p = path / fs::path(std::string("enemy") + std::to_string(i));
            if (!fs::exists(p)) {
                break;
            }
            FILE* file = fopen(p.string().c_str(), "r");
            opponents.push_back(loadEnemy(file));
            fclose(file);
        }
    } catch (const fs::filesystem_error& e) {
        return 1;
    }

    return 0;
}


/// loads all assets
/// @return 0 if succes, otherwise 1
int loadAssets()
{
    loadEnemyAssets();
    return loadMinigameAssets();
}
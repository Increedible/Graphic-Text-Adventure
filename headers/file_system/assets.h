#pragma once
#include <iostream>
#include <vector>
#include <filesystem>
#include "stage.h"

namespace fs = std::filesystem;

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
void saveImage(FILE *f, const image<n> &i)
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
    fprintf(f, "%d %d %d %d %d %d %d %d %d\n", out.hp, out.attackdurance, out.attackdmg, out.type,
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

// Helper to write a string with length prefix
void writeString(FILE* f, const std::string& str) {
    size_t len = str.size();
    fprintf(f, "%d\n", len);              // write length first
    fwrite(str.data(), 1, len, f);                // write raw bytes
}

// Helper to read a string with length prefix
std::string readStringf(FILE* f) {
    size_t len;
    fscanf(f, "%d\n", &len);               // read length
    std::string str(len, '\0');
    fread(&str[0], 1, len, f);                    // read raw bytes
    return str;
}

void saveStage(FILE* f, const Stage &s)
{
    // id
    fprintf(f, "%d\n", s.id);

    // name
    writeString(f, s.name.str);
    writeString(f, s.dialogue.str);
    writeString(f, s.conclusion.str);

    // directions
    for (int i = 0; i < 4; ++i)
        fprintf(f, "%d%c", s.directions[i], i < 3 ? ' ' : '\n');

    // images
    saveImage<16>(f, s.vis);
    saveImage<6>(f, s.legend);

    // needcoins and coinreward
    fprintf(f, "%d %d\n", s.needcoins, s.coinreward);

    // optional shop and minigame
    fprintf(f, "%d\n", s.shop.has_value() ? s.shop.value() : -1);
    fprintf(f, "%d\n", s.minigame.has_value() ? s.minigame.value() : -1);

    // encounter
    if (s.encounter.has_value()) {
        fprintf(f, "%d\n", s.encounter->first);
        writeString(f, s.encounter->second.str);
    } else {
        fprintf(f, "-1\n0\n"); // no encounter, zero-length string
    }

    // possible encounters
    fprintf(f, "%zu\n", s.possibleEncounters.size());
    for (const auto& [opponent, probability] : s.possibleEncounters)
        fprintf(f, "%d %.6f\n", opponent, probability);
}

Stage loadStage(FILE* f) {
    Stage s;

    // id
    fscanf(f, "%d\n", &s.id);

    // name, dialogue, conclusion
    s.name = readStringf(f);
    s.dialogue = readStringf(f);
    s.conclusion = readStringf(f);

    // directions
    fscanf(f, "%d %d %d %d\n", &s.directions[0], &s.directions[1], &s.directions[2], &s.directions[3]);

    // images
    loadImage<16>(f, s.vis);
    loadImage<6>(f, s.legend);

    // needcoins and coinreward
    fscanf(f, "%d %d\n", &s.needcoins, &s.coinreward);

    // shop and minigame
    int temp;
    fscanf(f, "%d\n", &temp);
    s.shop = (temp == -1) ? std::nullopt : std::optional<int>(temp);
    fscanf(f, "%d\n", &temp);
    s.minigame = (temp == -1) ? std::nullopt : std::optional<int>(temp);

    // encounter
    fscanf(f, "%d\n", &temp);
    if (temp == -1) {
        s.encounter = std::nullopt;
        fscanf(f, "%d\n", &temp); // skip length zero
    } else {
        s.encounter = std::pair<int, std::string>();
        s.encounter->first = temp;
        s.encounter->second = readStringf(f);
    }

    // possible encounters
    size_t n;
    fscanf(f, "%zu\n", &n);
    s.possibleEncounters.resize(n);
    for (size_t i = 0; i < n; ++i) {
        int opponent;
        double prob;
        fscanf(f, "%d %lf\n", &opponent, &prob);
        s.possibleEncounters[i] = {opponent, prob};
    }

    return s;
}


/// loads all assets
/// @return 0 if succes, otherwise 1
int loadAssets()
{
    loadEnemyAssets();
    return loadMinigameAssets();
}
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
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                FILE* file = fopen(entry.path().string().c_str(), "r");
                minigameVis.emplace_back();
                loadImage<23>(file, minigameVis.back());
            }
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
    return loadMinigameAssets();
}
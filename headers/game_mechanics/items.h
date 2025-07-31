#pragma once
#include <string>

struct item {
    int id;
    std::string name;
    std::string description;
    bool isdroppable;
    bool isusable;
    int healthrestore=0; // If isedible, how much health does it restore?
    item(int tid, const std::string& tname, const std::string& tdescription, bool tisdroppable, bool tisusable, int thealthrestore=0);
};

extern item items[];
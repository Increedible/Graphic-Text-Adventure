#include "items.h"

item::item(int tid, const std::string& tname, const std::string& tdescription, bool tisdroppable, bool tisusable, int thealthrestore) {
    id=tid;
    name=tname;
    description=tdescription;
    isdroppable=tisdroppable;
    tisusable=tisusable;
    healthrestore=thealthrestore;
}

item items[] = {
    {0, "NULL", "", false, false, false},
    {1, "Dusty Key", "A key that looks like it hasn't been used in a long time.", true, false, false},
    {2, "Premium Pass", "A pass that allows you to enter the club.", true, false, false},
    {3, "Wooden Planks", "A few wooden planks that can be used for crafting.", true, true, false},
    {4, "Red Balloon", "A red balloon that can be used to cheer up someone.", true, true, false},
    {5, "Oven Cookie", "A delicious cookie that restores some health when eaten.", true, false, 25},
};
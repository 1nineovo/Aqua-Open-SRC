#pragma once
#include "../../ModuleBase/Module.h"

class Swing : public Module {
public:
	int speed = 12;
    bool swingSpeed = false;
    bool fluxSwing = false;
    bool fakeBlock = false;
    bool shouldBlock = false;
    bool smallItem = false;
    Swing();
    void onDisable();
    void onEnable();
    void onNormalTick(LocalPlayer* lp);

    std::string getItemNameLower(Item* item) {
        if(!item)
            return "";
        std::string name = item->mName;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        return name;
    }
};

#pragma once
#include <chrono>

#include "../../ModuleBase/Module.h"

class ContainerScreenController;
class ItemStack;

class ChestStealer : public Module {
   public:
    ChestStealer();

    virtual void onEnable() override;
    virtual void onChestScreen(
        ContainerScreenController* csc) override;

    int delayValue = 100;
    bool randomizeDelay = true;
    int randomMin = 50;
    int randomMax = 150;
    bool autoClose = true;  
    bool smartSteal = true; 

   private:
    std::chrono::high_resolution_clock::time_point lastStealTime;
    long long currentDelay = 0;

    void resetTimer();
    bool checkDelay();
    long long generateNextDelay();
    bool isUselessItem(ItemStack* item);  
};
#pragma once
#include <chrono>
#include <queue>
#include <string>

#include "../../ModuleBase/Module.h"

enum class ActionType { DROP, EQUIP, SWAP };

struct InvAction {
    ActionType type;
    int slot;
    int targetSlot;
};

class InvManager : public Module {
   public:
    InvManager();
    virtual void onNormalTick(LocalPlayer* lp) override;

    bool dropUselessItems = true;
    bool autoEquipArmor = true;
    bool manageTools = true;
    bool keepBlocks = true;
    bool keepEnderPearls = true;
    bool keepFireProtection = false;
    float delayValue = 120.0f;  // 建议默认值调高一点，如120ms，解决伪物品
    bool silentMode = false;

    int preferredSwordSlot = 1;
    int preferredPickaxeSlot = 2;
    int preferredAxeSlot = 3;
    int preferredBlocksSlot = 9;

   private:
    std::queue<InvAction> actionQueue;
    std::chrono::high_resolution_clock::time_point lastActionTime;
    bool isInventoryOpen = false;

    int bestSlots[8] = {-1};

    void scanInventory(LocalPlayer* lp);
    void processQueue(LocalPlayer* lp);

    bool isUsefulItem(ItemStack* item);
    int getScore(ItemStack* item);
    std::string getItemNameLower(Item* item);
    int getSlotIdByName(const std::string& name);
};
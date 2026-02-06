#include "InvManager.h"
#include <Minecraft/InvUtil.h>
#include <algorithm>  // transform, tolower
#include "../../../../../../SDK/GlobalInstance.h"

InvManager::InvManager()
    : Module("InvManager", "Manages your inventory automatically", Category::PLAYER) {
    registerSetting(
        new BoolSetting("Drop Useless Items", "Drop useless items", &dropUselessItems, true));
    registerSetting(new BoolSetting("Auto Equip Armor", "Automatically equip better armor",
                                    &autoEquipArmor, true));
    registerSetting(
        new BoolSetting("Manage Tools", "Organize tools in hotbar", &manageTools, true));
    registerSetting(new BoolSetting("Keep Blocks", "Keep building blocks", &keepBlocks, true));
    registerSetting(
        new BoolSetting("Keep Ender Pearls", "Keep ender pearls", &keepEnderPearls, true));
    registerSetting(new SliderSetting<float>("Delay", "Delay (ms) - Fix Ghost Items", &delayValue,
                                             120.0f, 50.0f, 500.0f));
    registerSetting(new BoolSetting("Silent Mode", "Reduce chat messages", &silentMode, false));

    registerSetting(
        new SliderSetting<int>("Sword Slot", "Preferred sword slot", &preferredSwordSlot, 1, 0, 9));
    registerSetting(new SliderSetting<int>("Pickaxe Slot", "Preferred pickaxe slot",
                                           &preferredPickaxeSlot, 2, 0, 9));
    registerSetting(
        new SliderSetting<int>("Axe Slot", "Preferred axe slot", &preferredAxeSlot, 3, 0, 9));
    registerSetting(new SliderSetting<int>("Blocks Slot", "Preferred blocks slot",
                                           &preferredBlocksSlot, 9, 0, 9));
}

std::string InvManager::getItemNameLower(Item* item) {
    if(!item)
        return "";
    std::string name = item->mName;
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    return name;
}

int InvManager::getSlotIdByName(const std::string& name) {
    if(name.find("helmet") != std::string::npos)
        return 0;
    if(name.find("chestplate") != std::string::npos)
        return 1;
    if(name.find("leggings") != std::string::npos)
        return 2;
    if(name.find("boots") != std::string::npos)
        return 3;
    if(name.find("sword") != std::string::npos)
        return 4;
    if(name.find("pickaxe") != std::string::npos)
        return 5; 
    if(name.find("axe") != std::string::npos)
        return 6;
    if(name.find("shovel") != std::string::npos)
        return 7;
    return -1;
}

void InvManager::onNormalTick(LocalPlayer* lp) {
    if(!lp || !lp->getsupplies())
        return;

    std::string currentScreen = GI::getClientInstance()->getScreenName();
    bool currentlyOpen = (currentScreen == "inventory_screen");

    if(currentlyOpen && !isInventoryOpen) {
        isInventoryOpen = true;
        while(!actionQueue.empty())
            actionQueue.pop();
        scanInventory(lp);
        if(!silentMode)
            lp->displayClientMessage("§aInvManager: Sorting...");
    } else if(!currentlyOpen && isInventoryOpen) {
        isInventoryOpen = false;
        while(!actionQueue.empty())
            actionQueue.pop();
    }

    if(isInventoryOpen && !actionQueue.empty()) {
        processQueue(lp);
    }
}

void InvManager::scanInventory(LocalPlayer* lp) {
    for(int& s : bestSlots)
        s = -1;
    int bestScores[8] = {0};

    auto armorContainer = lp->getArmorContainer();
    for(int i = 0; i < 4; i++) {
        ItemStack* item = armorContainer->getItem(i);
        if(InvUtil::isVaildItem(item)) {
            bestScores[i] = getScore(item);
        }
    }

    for(int i = 0; i < 36; i++) {
        ItemStack* item = InvUtil::getItem(i);
        if(!InvUtil::isVaildItem(item))
            continue;

        Item* itemData = item->getItem();
        if(!itemData)
            continue;

        std::string name = getItemNameLower(itemData);
        int typeIdx = getSlotIdByName(name);

        if(typeIdx != -1) {
            int score = getScore(item);
            if(score > bestScores[typeIdx]) {
                bestScores[typeIdx] = score;
                bestSlots[typeIdx] = i;
            }
        }
    }

    if(autoEquipArmor) {
        for(int i = 0; i < 4; i++) {
            if(bestSlots[i] != -1) {
                actionQueue.push({ActionType::EQUIP, bestSlots[i], 0});
                bestSlots[i] = -1;
            }
        }
    }

    if(dropUselessItems) {
        for(int i = 0; i < 36; i++) {
            ItemStack* item = InvUtil::getItem(i);
            if(!InvUtil::isVaildItem(item))
                continue;

            bool isBest = false;
            for(int bs : bestSlots)
                if(i == bs)
                    isBest = true;
            if(isBest)
                continue;

            if(!isUsefulItem(item)) {
                actionQueue.push({ActionType::DROP, i, 0});
            }
        }
    }

    if(manageTools) {
        auto addSwap = [&](int bestIdx, int prefSlot) {
            if(prefSlot > 0 && bestSlots[bestIdx] != -1) {
                int target = prefSlot - 1;
                if(bestSlots[bestIdx] != target) {
                    actionQueue.push({ActionType::SWAP, bestSlots[bestIdx], target});
                    bestSlots[bestIdx] = target;
                }
            }
        };

        addSwap(4, preferredSwordSlot);
        addSwap(5, preferredPickaxeSlot);
        addSwap(6, preferredAxeSlot);

        if(preferredBlocksSlot > 0) {
            int target = preferredBlocksSlot - 1;
            ItemStack* current = InvUtil::getItem(target);
            if(!InvUtil::isVaildItem(current) || !current->isBlock()) {
                for(int i = 0; i < 36; i++) {
                    ItemStack* it = InvUtil::getItem(i);
                    if(InvUtil::isVaildItem(it) && it->isBlock()) {
                        bool isBestTool = false;
                        for(int s : bestSlots)
                            if(s == i)
                                isBestTool = true;

                        if(!isBestTool && i != target) {
                            actionQueue.push({ActionType::SWAP, i, target});
                            break;
                        }
                    }
                }
            }
        }
    }
}

void InvManager::processQueue(LocalPlayer* lp) {
    auto now = std::chrono::high_resolution_clock::now();
    auto msPassed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastActionTime).count();

    if(msPassed < delayValue)
        return;

    InvAction action = actionQueue.front();
    actionQueue.pop();

    ItemStack* item = InvUtil::getItem(action.slot);

    if(!InvUtil::isVaildItem(item) || item->mCount <= 0) {
        return;
    }

    bool executed = false;

    try {
        switch(action.type) {
            case ActionType::DROP:
                lp->getsupplies()->dropSlot(action.slot);
                if(!silentMode)
                    lp->displayClientMessage("Dropped slot " + std::to_string(action.slot));
                executed = true;
                break;

            case ActionType::EQUIP:
                lp->getsupplies()->equipArmor(action.slot);
                if(!silentMode)
                    lp->displayClientMessage("Equipped slot " + std::to_string(action.slot));
                executed = true;
                break;

            case ActionType::SWAP:
                lp->getsupplies()->swapSlots(action.slot, action.targetSlot);
                executed = true;
                break;
        }
    } catch(...) {
    }

    if(executed) {
        lastActionTime = now;
    }
}

int InvManager::getScore(ItemStack* item) {
    if(!InvUtil::isVaildItem(item))
        return -1;
    Item* data = item->getItem();
    if(!data)
        return -1;

    std::string name = getItemNameLower(data);
    int score = 0;

    if(name.find("netherite") != std::string::npos)
        score += 1000;
    else if(name.find("diamond") != std::string::npos)
        score += 800;
    else if(name.find("iron") != std::string::npos)
        score += 600;
    else if(name.find("golden") != std::string::npos)
        score += 400;
    else if(name.find("stone") != std::string::npos)
        score += 200;
    else
        score += 50;

    if(name.find("sword") != std::string::npos) {
        score += item->getEnchantValue(static_cast<int>(Enchant::SHARPNESS)) * 25;
    } else if(name.find("pickaxe") != std::string::npos || name.find("axe") != std::string::npos) {
        score += item->getEnchantValue(static_cast<int>(Enchant::EFFICIENCY)) * 20;
        score += item->getEnchantValue(static_cast<int>(Enchant::FORTUNE)) * 15;
    } else {
        score += item->getEnchantValue(static_cast<int>(Enchant::PROTECTION)) * 20;
        if(keepFireProtection &&
           item->getEnchantValue(static_cast<int>(Enchant::FIRE_PROTECTION)) > 0) {
            score += 2000;
        }
    }

    if(data->getMaxDamage() > 0) {
        float durability = 1.0f - ((float)item->getDamageValue() / (float)data->getMaxDamage());
        score += (int)(durability * 50.0f);
    }

    return score;
}

bool InvManager::isUsefulItem(ItemStack* item) {
    if(!InvUtil::isVaildItem(item))
        return false;
    Item* data = item->getItem();
    if(!data)
        return false;

    std::string name = getItemNameLower(data);

    if(keepBlocks && item->isBlock())
        return true;
    if(keepEnderPearls && name.find("ender_pearl") != std::string::npos)
        return true;

    if(name.find("flint") != std::string::npos)
        return true;
    if(name.find("shears") != std::string::npos)
        return true;
    if(name.find("bow") != std::string::npos)
        return true;
    if(name.find("shield") != std::string::npos)
        return true;
    if(name.find("rod") != std::string::npos)
        return true;

    if(name.find("apple") != std::string::npos)
        return true;
    if(name.find("totem") != std::string::npos)
        return true;
    if(name.find("elytra") != std::string::npos)
        return true;
    if(name.find("arrow") != std::string::npos)
        return true;
    if(name.find("bucket") != std::string::npos)
        return true;

    if(data->isFood()) {
        return true;
    }
    return false;
}
#include "ChestStealer.h"

#include <Minecraft/InvUtil.h>  // 假设你有物品工具

#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../SDK/World/Actor/LocalPlayer.h"
#include "../../../../../../SDK/World/Inventory/ContainerManagerModel.h"
#include "../../../../../../Utils/Maths.h" 

ChestStealer::ChestStealer()
    : Module("ChestStealer", "Steal items from chests automatically", Category::PLAYER) {
    registerSetting(new SliderSetting<int>("Delay", "Base delay (ms)", &delayValue, 100, 0, 1000));

    registerSetting(new BoolSetting("Randomize", "Randomize delay", &randomizeDelay, true));
    registerSetting(new SliderSetting<int>("RandMin", "Min random delay", &randomMin, 50, 0, 500));
    registerSetting(
        new SliderSetting<int>("RandMax", "Max random delay", &randomMax, 150, 0, 1000));

    registerSetting(new BoolSetting("Auto Close", "Close when empty", &autoClose, true));
    registerSetting(new BoolSetting("Smart Steal", "Only steal useful items", &smartSteal, true));
}

void ChestStealer::onEnable() {
    resetTimer();
}

void ChestStealer::resetTimer() {
    lastStealTime = std::chrono::high_resolution_clock::now();
    currentDelay = 0;
}

long long ChestStealer::generateNextDelay() {
    if(!randomizeDelay)
        return delayValue;
    int min = std::min(randomMin, randomMax);
    int max = std::max(randomMin, randomMax);
    return Math::randomInt(min, max);  
}

bool ChestStealer::checkDelay() {
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStealTime).count();

    if(elapsed >= currentDelay) {
        lastStealTime = now;
        currentDelay = generateNextDelay(); 
        return true;
    }
    return false;
}

void ChestStealer::onChestScreen(ContainerScreenController* csc) {
    auto player = GI::getLocalPlayer();
    if(!player || !csc)
        return;

    ContainerManagerModel* chestModel = player->getContainerManagerModel();

    if(!chestModel)
        return;


    if(chestModel->getmContainerType() != ContainerType::Container &&
       chestModel->getmContainerType() != ContainerType::ChestBoat) {
        return;
    }

    std::vector<int> slotsToSteal;
    bool isChestEmpty = true; 

    for(int i = 0; i < 54; i++) {
        ItemStack* stack = chestModel->getSlot(i);

        if(InvUtil::isVaildItem(stack) && stack->mItem) {
            if(smartSteal && isUselessItem(stack)) {
                continue;
            }

            isChestEmpty = false;
            slotsToSteal.push_back(i);
        }
    }

    if(isChestEmpty && autoClose) {
        csc->_tryExit(); 
        return;
    }

    if(!slotsToSteal.empty()) {
        if(checkDelay()) {
            int slot = slotsToSteal[0];
            csc->handleAutoPlace("container_items", slot);
        }
    }
}

bool ChestStealer::isUselessItem(ItemStack* item) {
    if(!item || !item->getItem())
        return true;

    Item* itemData = item->getItem();
    std::string name = itemData->mName; 
    if(item->isBlock())
        return false;

    SItemType type = itemData->getItemType();
    if(type == SItemType::Sword || type == SItemType::Shovel || type == SItemType::Helmet ||
       type == SItemType::Pickaxe || type == SItemType::Axe) {
        return false;
    }

    if(name.find("diamond") != std::string::npos)
        return false;
    if(name.find("iron") != std::string::npos)
        return false;
    if(name.find("gold") != std::string::npos)
        return false;
    if(name.find("emerald") != std::string::npos)
        return false;
    if(name.find("ender_pearl") != std::string::npos)
        return false;
    if(name.find("apple") != std::string::npos)
        return false;  
    if(name.find("tnt") != std::string::npos)
        return false;
    if(name.find("arrow") != std::string::npos)
        return false;
    return true;
}
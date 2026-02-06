#include <Minecraft/TargetUtil.h>
#include <Minecraft/WorldUtil.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <mutex>
#include <random>
#include <thread>

#include "../../../ModuleManager.h"
#include "../Render/SetColor.h"
#include "Aura.h"
#include <Minecraft/InvUtil.h>
int PacketA = 1;
int weaponSwitchCounter = 0;
Aura::Aura() : Module("Aura", "Killaura for IGN servers", Category::COMBAT) {
    registerSetting(
        new EnumSetting("Rotations", "Rotations Mode", {"None", "Slient", "Actual"}, &rotMode, 0));
    registerSetting(new SliderSetting<float>("Range", "Range in which targets will be hit", &range,
                                             5.f, 3.f, 10.f));
    registerSetting(new SliderSetting<float>("WallRange",
                                             "Range in which targets will be hit through walls",
                                             &wallRange, 0.f, 0.f, 150.f));
    registerSetting(new EnumSetting("Mode", "NULL", {"Single", "Multi"}, &mode, 0));
    registerSetting(new SliderSetting<int>("APS", "numbers of attack per tick", &APS, 1, 0, 20));
    registerSetting(new SliderSetting<int>("Delay", "Attack delay (Tick)", &delay, 1, 0, 20));
    registerSetting(new EnumSetting("Weapon", "Auto switch to best weapon",
                                    {"None", "Switch", "Spoof"}, &autoWeaponMode, 0));
    registerSetting(new BoolSetting("Mobs", "Attack Mobs", &includeMobs, false));
}

std::string Aura::getModeText() {
    switch(mode) {
        case 0: {
            return "Single";
            break;
        }
        case 1: {
            return "Multi";
            break;
        }
    }
    return "NULL";
}
int getBestWeapon(Actor* target) {
    auto* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return -1;  
    auto* plrInv = localPlayer->getsupplies();
    if(!plrInv)
        return -1;  
    auto* inv = plrInv->container;
    if(!inv)
        return -1;  
    int bestSlot = InvUtil::getSelectedSlot(); 
    float bestValue = 0.f;
    for(auto i = 0; i < 9; i++) {
        auto* itemStack = inv->getItem(i);
        if(!itemStack || !InvUtil::isVaildItem(itemStack))
            continue;  
        
        Item* itemData = itemStack->getItem();
        if (!itemData) continue;
        
        if (!itemData->isSword() && !itemData->isAxe()) {
            continue;
        }
        
        int tier = itemData->getItemTier();
        float currentValue = tier * 80;
        
        currentValue += itemStack->getEnchantValue(static_cast<int>(Enchant::SHARPNESS)) * 15;
        if (itemData->getMaxDamage() > 0) {
            short damage = itemStack->getDamageValue();
            float durabilityPercent = 1.0f - (float(damage) / itemData->getMaxDamage());
            if (durabilityPercent < 0.5f) {
                currentValue = currentValue * 0.7f;
            }
        }
        
        if(currentValue > bestValue) {
            bestValue = currentValue;
            bestSlot = i;
        }
    }
    return bestSlot;
}


void Aura::onEnable() {
    targetList.clear();
}

void Aura::onDisable() {
    targetList.clear();
}

bool Aura::sortByDist(Actor* a1, Actor* a2) {
    Vec3<float> lpPos = GI::getLocalPlayer()->getPos();
    return ((a1->getPos().dist(lpPos)) < (a2->getPos().dist(lpPos)));
}

int Aura::getBestWeaponSlot(Actor* target) {}

void Aura::Attack(Actor* target) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer || !localPlayer->getgamemode())
        return;

    GameMode* gm = localPlayer->getgamemode();

    gm->attack(target);

    localPlayer->swing();
}

void Aura::onNormalTick(LocalPlayer* localPlayer) {
    Level* level = localPlayer->level;
    BlockSource* region = GI::getRegion();

    targetList.clear();

    for(auto& entity : level->getRuntimeActorList()) {
        if(TargetUtil::isTargetValid(entity, includeMobs)) {
            float rangeCheck = range;
            if(region->getSeenPercent(localPlayer->getEyePos(),
                                      entity->getAABBShapeComponent()->getAABB()) == 0.0f)
                rangeCheck = wallRange;

            if(WorldUtil::distanceToEntity(localPlayer->getPos(), entity) <= rangeCheck)
                targetList.push_back(entity);
        }
    }

    std::sort(targetList.begin(), targetList.end(), sortByDist);

    if(targetList.empty()) {
        shouldRot = false;
        return;
    }

    if(autoWeaponMode > 0 && !targetList.empty()) {
        int bestWeaponSlot = getBestWeapon(targetList[0]);
        if(bestWeaponSlot != -1 && bestWeaponSlot != InvUtil::getSelectedSlot()) {
            if(autoWeaponMode == 1) {
                InvUtil::switchSlot(bestWeaponSlot);
            } else if(autoWeaponMode == 2) {
                InvUtil::sendMobEquipment(bestWeaponSlot);
            }
        }
    }

    Vec3<float> aimPos = targetList[0]->getEyePos();
    aimPos.y = targetList[0]->getAABBShapeComponent()->getAABB().lower.y;
    rotAngle = localPlayer->getEyePos().CalcAngle(aimPos);
    rotAngle5 = localPlayer->getEyePos().CalcAngle(aimPos);
    Vec2<float> ange = localPlayer->getPos().CalcAngle(targetList[0]->getPos()).normAngles();

    shouldRot = true;

    if(oTick >= delay) {
        if(mode == 0) {
            for(auto& target : targetList) {
                Attack(target);
            }
            oTick = 0;
        } else if(mode == 1) {
            for(auto& target : targetList) {
                for(int i = 0; i < APS; i++) {
                    Attack(target);
                }
            }
            oTick = 0;
        }
    } else {
        oTick++;
    }
}
void Aura::onUpdateRotation(LocalPlayer* localPlayer) {
    if(!shouldRot)
        return;

    ActorRotationComponent* rotation = localPlayer->getActorRotationComponent();
    ActorHeadRotationComponent* headRot = localPlayer->getActorHeadRotationComponent();
    MobBodyRotationComponent* bodyRot = localPlayer->getMobBodyRotationComponent();

    switch(rotMode) {
        case 0: {
            return;
            break;
        }
        case 1: {
            if(rotation) {
                rotation->mOldPitch = rotAngle5.x;
                rotation->mOldYaw = rotAngle5.y;
            }
            break;
        }
        case 2: {
            if(rotation) {
                rotation->mYaw = rotAngle5.y;
                rotation->mPitch = rotAngle5.x;
                rotation->mOldPitch = rotAngle5.x;
                rotation->mOldYaw = rotAngle5.y;
            }
            if(headRot) {
                headRot->mHeadRot = rotAngle5.y;
            }
            if(bodyRot) {
                bodyRot->yBodyRot = rotAngle5.y;
            }
            break;
        }
    }
}


void Aura::onLevelRender() {

    if(targetList.empty())
        return;

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

    float speed = 0.003f; 
    float height = 2.0f; 
    float radius = 0.6f;  

    float animFactor = (sin(ms * speed) + 1.0f) * 0.5f;

    float currentHeight = animFactor * height;

    UIColor ringColor(255, 60, 60, 255);

    for(auto* target : targetList) {
        if(!target)
            continue;

        Vec3<float> pos = target->getPos();

        pos.y += currentHeight;

        DrawUtil::drawRing3d(pos, radius, ringColor, 2.0f);
    }
}
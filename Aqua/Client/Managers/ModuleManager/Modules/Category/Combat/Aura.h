#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <random>
#include <thread>

#include "../../ModuleBase/Module.h"

class Aura : public Module {
   private:
    int rotMode = 0;
    bool includeMobs = false;
    float range = 5.0f;
    float wallRange = 0.0f;
    int mode = 0;
    int APS = 1;
    int delay = 1;
    int oTick = 0;
    int autoWeaponMode = 0;
    std::vector<Actor*> targetList;
    Vec2<float> rotAngle;
    Vec2<float> rotAngle5;
    bool shouldRot = false;

    float circleAngle = 0.0f;
    float circleRadius = 0.5f;
    float circleSpeed = 0.1f;

    void Attack(Actor* target);
    static bool sortByDist(Actor* a1, Actor* a2);
    int getBestWeaponSlot(Actor* target);

   public:
    Aura();

    std::string getModeText();

    virtual void onEnable() override;
    virtual void onDisable() override;
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
    virtual void onUpdateRotation(LocalPlayer* localPlayer) override;
    void onLevelRender() override;
};
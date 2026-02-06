#pragma once
#pragma once
#include "../../../../../../Utils/Minecraft/DamageUtil.h"
#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"

struct CrystalFadeStruct {
    Vec3<float> lastPos;
    float fadeTimer;
    float fadeDuration;
};

struct CrystalData {
   public:
    float targetDamage;
    float localDamage;
    Actor* actor;

   protected:
    static float getExplosionDamage(const Vec3<float>& blockPos, Actor* _actor) {
        return DamageUtils::getExplosionDamage(blockPos, _actor);
    }
};

class CrystalPlace : public CrystalData {
   public:
    BlockPos blockPos;
    CrystalPlace(Actor* _actor, const BlockPos& _blockPos) {
        blockPos = _blockPos;
        Vec3<float> crystalPos =
            Vec3<float>(BlockPos(_blockPos).toFloat())
                .add((0.5f, 1.f, 0.5f));  // end crystal is entity not block :joebig:
        targetDamage = getExplosionDamage(crystalPos, _actor);
        localDamage = getExplosionDamage(crystalPos, GI::getLocalPlayer());
        actor = _actor;
    }
};

class CrystalBreak : public CrystalData {
   public:
    Actor* crystal;
    CrystalBreak(Actor* _actor, Actor* _crystal) {
        Vec3<float> crystalPos = _crystal->getEyePos();
        actor = _actor;
        crystal = _crystal;
        targetDamage = getExplosionDamage(crystalPos, _actor);
        localDamage = getExplosionDamage(crystalPos, GI::getLocalPlayer());
    }
};

class AutoCrystal : public Module {
   public:
    std::vector<CrystalPlace> placeList;
    std::vector<CrystalBreak> breakList;
    std::vector<Actor*> targetList;
    std::vector<Actor*> entityList;
    std::vector<CrystalFadeStruct> fadeList;

   public:
    Vec3<float> lastPlaceLoc;
    std::unordered_map<int, Vec3<float>> lerpData;
    float decreasingAlpha = 0.f;
    int highestId = -1;
    bool shouldChange = false;
    int iBoostDelay = 0;
    int iPlaceDelay = 0;
    int iBreakDelay = 0;

   public:
    bool place = true;
    bool abreak = true;
    bool eatstop = true;
    bool antiTotem = false;
    int placeRange = 6;
    int breakRange = 6;
    int targetRange = 12;
    float proximity = 6.f;
    float enemyDamage = 4.f;
    float localDamage = 12.f;
    int wasteAmount = 1;

    bool facePlace = false;
    bool rotate = false;
    int placeDelay = 5;
    int breakDelay = 5;
    int predictSpeed = 20;
    bool predict = false;
    int predictPacket = 5;
    bool extrapolate = false;

   private:
    int switchMode = 2;
    int placeDelayTick = 0;
    int breakDelayTick = 0;
    int sendDelayTick = 0;

    int sendDelay = 0;

    float breakLocalDamage = 12.f;  // Max self damage when breaking
    float breakEnemyDamage = 4.f;   // Min enemy damage when breaking
    int renderType = 0;
    bool dmgText = false;
    bool selfTest = false;
    int animType = 0;
    float slideSpeed = 0.4f;
    float fadeDur = 1.f;
    UIColor renderColor = UIColor(255, 255, 255, 60);

   protected:
    static bool sortCrystal(CrystalData c1, CrystalData c2);
    bool isPlaceValid(const BlockPos& blockPos, Actor* actor);
    void generatePlacement(Actor* actor);

    void getCrystals(Actor* actor);

   protected:
    void placeCrystal(GameMode* gm);
    void breakCrystal(GameMode* gm);

   public:
    AutoCrystal();
    void breakIdPredictCrystal(GameMode* gm);
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
    virtual void onSendPacket(Packet* packet) override;
};
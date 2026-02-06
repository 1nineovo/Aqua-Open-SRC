#pragma once
#pragma once
#include "../../ModuleBase/Module.h"

class AutoMine : public Module {
   public:
    std::vector<Actor*> actorList;

   private:
    float range = 6.f;
    bool face = false;
    bool burrow = false;
    bool priority = false;

   private:
    BlockPos getMinePos(Actor* actor) const;

   public:
    AutoMine();
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
};
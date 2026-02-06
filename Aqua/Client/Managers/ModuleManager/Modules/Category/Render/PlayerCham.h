#pragma once
#include "../../ModuleBase/Module.h"

class PlayerCham : public Module {
   private:
    Vec3<float> headHalf = Vec3<float>(0.25, 0.25f, 0.25f);
    Vec3<float> bodyHalf = Vec3<float>(0.3f, 0.35f, 0.15f);
    Vec3<float> armHalf = Vec3<float>(0.15f, 0.35f, 0.15f);
    Vec3<float> legHalf = Vec3<float>(0.15f, 0.35f, 0.15f);

    Vec3<float> localLeftOffset1 = Vec3<float>(-0.4f, 0.7f, 0.0f);
    Vec3<float> localRightOffset1 = Vec3<float>(+0.4f, 0.7f, 0.0f);

    Vec3<float> localRighLegOffset = Vec3<float>(+0.15f, 0.f, 0.0f);
    Vec3<float> localLeftLegOffset = Vec3<float>(-0.15f, 0.f, 0.0f);

   public:
    bool isdancing = true;
    bool isFirstPerson = true;
    bool isLocal = false;
    UIColor col = UIColor(255, 255, 0, 180);
    UIColor linecol = UIColor(255, 255, 0, 180);
    PlayerCham();

    void onLevelRender() override;
};

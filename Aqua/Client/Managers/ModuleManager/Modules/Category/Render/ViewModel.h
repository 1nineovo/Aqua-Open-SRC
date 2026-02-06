#pragma once
#include "../../ModuleBase/Module.h"
class ViewModel : public Module {
   public:
    glm::mat4x4 oldMatrix;
    Vec3<float> mhTrans = Vec3<float>(0.f, 0.f, 0.f);
    Vec3<float> mhScale = Vec3<float>(1.f, 1.f, 1.f);

    Vec3<float> mhRot = Vec3<float>(0.f, 0.f, 0.f);
    EnumSetting* swingAnimType = nullptr;
    UIColor glintColor = UIColor(255, 255, 255, 255);  // Default color for animations
    bool isGlint = false;  // Flag to indicate if glint effect is enabled
   public:
    bool AttackAnim = false;
    bool Reset = false;
    float swingSpeed = 0.5f;
    float eatSpeed = 0.5f;
   private:



   public:
    ViewModel();
    virtual void onClientTick() override;
    Vec3<float> getFancySwingAnimation();
    Vec3<float> getFancyEatAnimation();
    Vec3<float> getFancyEatTranslation();
};
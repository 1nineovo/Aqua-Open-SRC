#pragma once
#pragma once
#include "../../ModuleBase/Module.h"

class CrystalChams : public Module {
   private:
    UIColor color = UIColor(255, 0, 0, 100);
    float speed = 1.0f;
    float upDownSpeed = 1.0f;   // Speed of the up-down movement
    float heightOffset = 0.5f;  // Maximum vertical offset

    float range = 16.0f;
    float scale = 0.5f;
    bool fill = true;

    int lineAlpha = 255;

   public:
    CrystalChams();

    void onLevelRender() override;
};
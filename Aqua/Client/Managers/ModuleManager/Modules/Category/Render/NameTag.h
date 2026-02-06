#pragma once
#include "../../ModuleBase/Module.h"

class NameTag : public Module {
   public:
    NameTag();
    float minScale = 0.4f;
    float maxScale = 1.5f;
    float scaleFactor = 6.0f;
    float opacity = 0.4f;

    void onImGuiRender(ImDrawList* drawList) override;
    float calculateScale(float distance);
    std::string sanitizeText(const std::string& text);
    void renderPlayerNameTag(Actor* actor);

};
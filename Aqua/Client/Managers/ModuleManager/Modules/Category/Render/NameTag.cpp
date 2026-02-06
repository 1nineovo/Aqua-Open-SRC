#include "NameTag.h"
#include "../../../../../../Utils/Minecraft/TargetUtil.h"
#include <ImGuiUtils.h>
#include "../../../../../../SDK/Render/Matrix.h"

NameTag::NameTag() : Module("NameTag", "Better nametags using RenderUtil.", Category::RENDER) {
    registerSetting(new SliderSetting<float>("Opacity", "NULL",
                                             &opacity, 0.4f, 0.1f, 1.0f));
    registerSetting(new SliderSetting<float>("MinScale", "Minimum scale for distant tags",
                                             &minScale, 0.4f, 0.1f, 1.0f));
    registerSetting(new SliderSetting<float>("MaxScale", "Maximum scale for close tags", &maxScale,
                                             1.5f, 1.0f, 3.0f));
    registerSetting(new SliderSetting<float>("ScaleFactor", "Scale calculation factor",
                                             &scaleFactor, 6.0f, 1.0f, 15.0f));
}

void NameTag::onImGuiRender(ImDrawList* drawList) {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;
    if(!GI::canUseMoveKeys())
        return;
    if(!GI::isInHudScreen())
        return;
    for(Actor* actor : localPlayer->level->getRuntimeActorList()) {
       
        if(!TargetUtil::isTargetValid(actor, false))
            continue;
        if(!actor || !actor->isAlive())
            continue;

        renderPlayerNameTag(actor);
    }
}

float NameTag::calculateScale(float distance) {
    if(distance <= 1.0f) {
        return maxScale;
    }

    float logScale = scaleFactor / std::log(distance * 1.5f + 1.0f);

    return std::clamp(logScale, minScale, maxScale);
}

static bool isInvalidChar(char c) {
    return !(c >= 0 && *reinterpret_cast<unsigned char*>(&c) < 128);
}

std::string NameTag::sanitizeText(const std::string& text) {
    std::string out;
    bool wasValid = true;
    for(char c : text) {
        bool isValid = !isInvalidChar(c);
        if(wasValid) {
            if(!isValid) {
                wasValid = false;
            } else {
                out += c;
            }
        } else {
            wasValid = isValid;
        }
    }
    return out;
}



void NameTag::renderPlayerNameTag(Actor* actor) {
    Vec2<float> screenPos;
    Vec3<float> worldPos = actor->getEyePos().add(Vec3<float>(0.f, 0.75f, 0.f));
    if(!Matrix::WorldToScreen(worldPos, screenPos))
        return;

    Vec2<float> windowSize = GI::getGuiData()->windowSizeReal;
    if(screenPos.x < -200 || screenPos.y < -200 || screenPos.x > windowSize.x + 200 ||
       screenPos.y > windowSize.y + 200) {
        return;
    }

    std::string tagPtr = actor->getNameTag();
    if(!&tagPtr)
        return;
    std::string rawName = sanitizeText(tagPtr);
    if(rawName.empty())
        rawName = "Player";

    LocalPlayer* localPlayer = GI::getLocalPlayer();
    float dist = actor->getEyePos().dist(localPlayer->getEyePos());
    float baseScale = calculateScale(dist);
        float textSize = 1.f * baseScale;
        float textWidth = ImGuiUtils::getTextWidth(rawName, textSize);
        float textHeight = ImGuiUtils::getTextHeight(textSize);
        float textPadding = 1.f * textSize;
        Vec2<float> textPos =
            Vec2<float>(screenPos.x - textWidth / 2.f, screenPos.y - textHeight / 2.f);
        Vec4<float> rectPos = Vec4<float>(textPos.x - textPadding * 2.f, textPos.y - textPadding,
                                          textPos.x + textWidth + textPadding * 2.f,
                                          textPos.y + textHeight + textPadding);
        Vec4<float> underline =
            Vec4<float>(rectPos.x, rectPos.w - 1.f * textSize, rectPos.z, rectPos.w);
        ImGuiUtils::fillRectangle(rectPos, UIColor(0, 0, 0, (int)(255 * opacity)));
        ImGuiUtils::fillRectangle(underline, UIColor(255, 255, 255, 255));
        ImGuiUtils::drawText(textPos, rawName, UIColor(255, 255, 255, 255), textSize, true);
}
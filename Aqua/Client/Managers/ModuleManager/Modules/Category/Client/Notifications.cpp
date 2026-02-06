#include "Notifications.h"

#include <ImGuiUtils.h>

#include <algorithm>
#include <string>

#include "../../../ModuleManager.h"

Notifications::Notifications() : Module("Notifications", "Show notification", Category::CLIENT) {}
Notifications::~Notifications() {
    notifList.clear();
}
bool Notifications::isVisible() {
    return false;
}

void Notifications::addNotifBox(std::string message, float duration) {
    for(auto& notif : notifList) {
        if(notif->message == message) {
            notif->count++;
            notif->duration = duration;
            notif->maxDuration = duration;
            return;
        }
    }

    std::shared_ptr<NotificationBox> notif = std::make_shared<NotificationBox>(message, duration);
    notif->count = 1;  // 初始计数为1

    if(GI::getGuiData()) {
        notif->PosX = GI::getGuiData()->windowSizeReal.x + 10.f;
        notif->PosY = GI::getGuiData()->windowSizeReal.y;
    }
    notifList.push_back(notif);
}

void Notifications::Render(ImDrawList* drawlist) {
    static Notifications* module = ModuleManager::getModule<Notifications>();
    if(!module->isEnabled()) {
        if(!notifList.empty())
            notifList.clear();
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    float deltaTime = io.DeltaTime;
    Vec2 screenSize = GI::getGuiData()->windowSizeReal;

    const float textSize = 1.0f;
    const float countTextSize = 0.9f;  // 计数文字稍小
    const float paddingX = 10.0f;
    const float paddingY = 7.0f;
    const float rounding = 4.0f;  // 统一圆角
    const float gap = 6.0f;       // 卡片间距

    float textHeight = ImGuiUtils::getTextHeight(textSize);
    float rectHeight = textHeight + (paddingY * 2.0f);

    float currentYAnchor = screenSize.y - 50.0f;

    static Interface* colorsMod = ModuleManager::getModule<Interface>();

    for(int i = 0; i < notifList.size(); ++i) {
        std::shared_ptr<NotificationBox>& notif = notifList[i];

        std::string displayText = notif->message;

        std::string countStr = "";
        bool hasCount = notif->count > 1;
        if(hasCount) {
            countStr = " x" + std::to_string(notif->count);
        }

        float textW = ImGuiUtils::getTextWidth(displayText, textSize);
        float countW = 0.0f;
        if(hasCount) {
            countW = ImGuiUtils::getTextWidth(countStr, countTextSize) +
                     6.0f;  // 计数文字宽度 + 左右内边距
        }

        float rectWidth = textW + countW + (paddingX * 2.0f);

        bool isExpiring = (notif->duration <= 0.0f);

        float targetX = isExpiring ? (screenSize.x + 20.0f) : (screenSize.x - rectWidth - 10.0f);
        float targetY = currentYAnchor - rectHeight;

        float animSpeed = 10.0f;
        notif->PosX = Math::lerp(notif->PosX, targetX, deltaTime * animSpeed);
        notif->PosY = Math::lerp(notif->PosY, targetY, deltaTime * 12.0f);

        if(notif->duration > 0.0f) {
            notif->duration -= deltaTime;
        } else {
            if(notif->PosX > screenSize.x) {
                notifList.erase(notifList.begin() + i);
                i--;
                continue;
            }
        }

        currentYAnchor -= (rectHeight + gap);

        if(notif->PosX > screenSize.x)
            continue;

        float dist = notif->PosX - targetX;
        float alphaFactor = 1.0f - std::clamp(dist / 50.0f, 0.0f, 1.0f);
        if(isExpiring)
            alphaFactor = 1.0f;

        int alpha = (int)(255 * alphaFactor);
        if(alpha < 5)
            continue;

        Vec4 rectPos(notif->PosX, notif->PosY, notif->PosX + rectWidth, notif->PosY + rectHeight);
        UIColor themeColor = colorsMod->getColor();

        drawlist->AddRectFilled(ImVec2(rectPos.x + 2, rectPos.y + 2),
                                ImVec2(rectPos.z + 2, rectPos.w + 2),
                                IM_COL32(0, 0, 0, (int)(40 * alphaFactor)), rounding);

        drawlist->AddRectFilled(ImVec2(rectPos.x, rectPos.y), ImVec2(rectPos.z, rectPos.w),
                                IM_COL32(20, 20, 25, (int)(140 * alphaFactor)), rounding);

        drawlist->AddRect(ImVec2(rectPos.x, rectPos.y), ImVec2(rectPos.z, rectPos.w),
                          IM_COL32(60, 60, 70, (int)(80 * alphaFactor)), rounding, 0, 1.0f);

        if(notif->maxDuration > 0.f) {
            float progress = notif->duration / notif->maxDuration;
            float barW = (rectWidth - 4.0f) * progress;

            drawlist->AddRectFilled(ImVec2(rectPos.x + 2.0f, rectPos.w - 2.5f),
                                    ImVec2(rectPos.z - 2.0f, rectPos.w - 0.5f),
                                    IM_COL32(40, 40, 45, (int)(120 * alphaFactor)), 1.0f);

            drawlist->AddRectFilled(ImVec2(rectPos.x + 2.0f, rectPos.w - 2.5f),
                                    ImVec2(rectPos.x + 2.0f + barW, rectPos.w - 0.5f),
                                    themeColor.toImColor((int)(180 * alphaFactor)), 1.0f);
        }

        float textY = notif->PosY + (rectHeight - textHeight) / 2.0f;
        float textX = notif->PosX + paddingX;

        ImGuiUtils::drawText(Vec2(textX + 1, textY + 1), displayText,
                             UIColor(0, 0, 0, (int)(100 * alphaFactor)), textSize, false);

        ImGuiUtils::drawText(Vec2(textX, textY), displayText, UIColor(240, 240, 245, alpha),
                             textSize, false);

        if(hasCount) {
            float countX = textX + textW + 3.0f;  // 主文字后面留3px间距
            float countTextH = ImGuiUtils::getTextHeight(countTextSize);
            float countY = notif->PosY + (rectHeight - countTextH) / 2.0f;

            UIColor themeColorText = themeColor;
            themeColorText.a = (int)(200 * alphaFactor);

            ImGuiUtils::drawText(Vec2(countX, countY), countStr, themeColorText, countTextSize,
                                 false);
        }
    }
}
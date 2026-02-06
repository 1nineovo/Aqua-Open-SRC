#include "Interface.h"

#include <AnimationUtil.h>
#include <DrawUtil.h>
#include <ImGuiUtils.h>
#include <MemoryUtil.h>
#include <Minecraft/InvUtil.h>
#include <SkinGameData.h>
#include <imfx.h>

#include <algorithm>
#include <cmath>

#include "../../../../HooksManager/Hooks/Network/GetAvgPingHook.h"
#include "../../../ModuleManager.h"
#include "Editor.h"

Interface::Interface() : Module("Interface", "UI thing", Category::CLIENT) {
    registerSetting(new BoolSetting("Hotbar", "show custom hotbar", &hotbar, true));
    registerSetting(new BoolSetting("Effect", "show play info", &effect, true));
    registerSetting(new BoolSetting("Keystrokes", "show play info", &keystrokes, true));

    registerSetting(new BoolSetting("Show Ping", "Display network ping", &showPing, true));
    registerSetting(new BoolSetting("Show FPS", "Display frames per second", &showFps, true));
    registerSetting(new BoolSetting("Show XYZ", "Display player coordinates", &showXyz, true));
    registerSetting(
        new BoolSetting("Show Direction", "Display facing direction", &showDirection, true));

    registerSetting(new EnumSetting("Mode",
                                    "Color mode\nSingle: Uses 1 color only\nWave: Transitions "
                                    "between 2 seperate colors\nRGB: Red Green Blue transition",
                                    {"Single", "Wave", "RGB"}, &colorType, 0));

    registerSetting(new ColorSetting("Color", "Client color", &mainColor, mainColor, false,
                                     [&]() -> bool { return (colorType == 0); }));
    registerSetting(new ColorSetting("Primary", "Client color", &primaryColor, primaryColor, false,
                                     [&]() -> bool { return (colorType == 1); }));
    registerSetting(new ColorSetting("Secondary", "Client color", &secondColor, secondColor, false,
                                     [&]() -> bool { return (colorType == 1); }));

    registerSetting(new SliderSetting<float>("Seconds", "RGB Interval in seconds", &seconds,
                                             seconds, 1.f, 10.f,
                                             [&]() -> bool { return (colorType == 2); }));
    registerSetting(new SliderSetting<float>("Saturation", "RGB Saturation", &saturation,
                                             saturation, 0.f, 1.f,
                                             [&]() -> bool { return (colorType == 2); }));
    registerSetting(new SliderSetting<float>("Brightness", "RGB Brightness", &brightness,
                                             brightness, 0.f, 1.f,
                                             [&]() -> bool { return (colorType == 2); }));
    registerSetting(new SliderSetting<int>("Seperation", "RGB Seperation", &seperation, seperation,
                                           1, 200, [&]() -> bool { return (colorType == 2); }));
}

void Interface::onEnable() {
    static Editor* editor = ModuleManager::getModule<Editor>();
    if(editor) {
        editor->registerWidget("potion_effects", [](Vec2<float> pos, float size) {});
        editor->registerWidget("keystrokes", [](Vec2<float> pos, float size) {});
        editor->registerWidget("hud_info", [](Vec2<float> pos, float size) {});
    }
}

void Interface::onDisable() {}

void Interface::onNormalTick(LocalPlayer* lp) {
    if(!lp)
        return;
    Level* level = lp->level;
    if(!level)
        return;
}

void Interface::onD2DRender() {}

void Interface::onImGuiRender(ImDrawList* drawList) {
    Vec2<float> windowsSize = GI::getGuiData()->windowSizeReal;
    auto localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    static ClickGui* guiMod = ModuleManager::getModule<ClickGui>();
    if(guiMod->isEnabled())
        return;

    if(effect) {
        renderPotionEffects(windowsSize, localPlayer, drawList);
    }
    if(hotbar) {
        renderHotbar();
    }
    if(keystrokes) {
        renderKeystrokes(drawList);
    }
    if(showPing || showFps || showXyz || showDirection) {
        renderHudInfo(drawList);
    }

    UIColor currentThemeColor = this->getColor();
    for(auto& mod : ModuleManager::moduleList) {
        for(auto& setting : mod->getSettingList()) {
            if(setting->type == SettingType::COLOR_S) {
                ColorSetting* cs = static_cast<ColorSetting*>(setting);
                if(cs->theme) {
                    *cs->colorPtr = currentThemeColor;
                }
            }
        }
    }
}

std::string Interface::getDirectionString(float yaw) {
    while(yaw < 0)
        yaw += 360.0f;
    while(yaw >= 360)
        yaw -= 360.0f;

    if(yaw >= 337.5f || yaw < 22.5f)
        return "S";
    else if(yaw >= 22.5f && yaw < 67.5f)
        return "SW";
    else if(yaw >= 67.5f && yaw < 112.5f)
        return "W";
    else if(yaw >= 112.5f && yaw < 157.5f)
        return "NW";
    else if(yaw >= 157.5f && yaw < 202.5f)
        return "N";
    else if(yaw >= 202.5f && yaw < 247.5f)
        return "NE";
    else if(yaw >= 247.5f && yaw < 292.5f)
        return "E";
    else
        return "SE";
}

UIColor lerpColor(const UIColor& c1, const UIColor& c2, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return UIColor((int)(c1.r + (c2.r - c1.r) * t), (int)(c1.g + (c2.g - c1.g) * t),
                   (int)(c1.b + (c2.b - c1.b) * t), (int)(c1.a + (c2.a - c1.a) * t));
}

void Interface::renderHudInfo(ImDrawList* drawList) {
    std::string screenName = GI::getClientInstance()->getScreenName();
    if(screenName != "hud_screen")
        return;

    auto localPlayer = GI::getLocalPlayer();
    static Editor* editor = ModuleManager::getModule<Editor>();
    if(!editor)
        return;

    Vec2<float> hudPos = editor->getWidgetPosition("hud_info");
    float hudSize = editor->getWidgetSize("hud_info");

    std::vector<std::string> infoLines;

    if(showPing) {
        int ping = GetAveragePingHook::getLastPing();
        std::string pingStr = "Ping: " + (ping > 0 ? std::to_string(ping) + "ms" : "N/A");
        infoLines.push_back(pingStr);
    }

    if(showFps) {
        int fps = (int)(1.0f / ImGui::GetIO().DeltaTime);
        std::string fpsStr = "FPS: " + std::to_string(fps);
        infoLines.push_back(fpsStr);
    }

    if(showXyz && localPlayer) {
        Vec3<float> pos = localPlayer->getPos();
        char xyzBuf[64];
        snprintf(xyzBuf, sizeof(xyzBuf), "XYZ: %.1f, %.1f, %.1f", pos.x, pos.y - 1.62f, pos.z);
        infoLines.push_back(std::string(xyzBuf));
    }

    if(showDirection && localPlayer) {
        ActorRotationComponent* rotComp = localPlayer->getActorRotationComponent();
        if(rotComp) {
            float yaw = rotComp->mYaw;
            std::string dirStr = "Dir: " + getDirectionString(yaw);
            infoLines.push_back(dirStr);
        }
    }

    if(infoLines.empty())
        return;

    float textSize = 1.0f * hudSize;
    float padding = 8.0f * hudSize;
    float lineSpacing = 3.0f * hudSize;
    float cornerRadius = 4.0f * hudSize;

    float textHeight = ImGuiUtils::getTextHeight(textSize);

    float maxWidth = 0.0f;
    float totalHeight = 0.0f;

    for(const auto& line : infoLines) {
        float lineWidth = ImGuiUtils::getTextWidth(line, textSize);
        maxWidth = std::max(maxWidth, lineWidth);
        totalHeight += textHeight;
    }
    if(infoLines.size() > 1) {
        totalHeight += lineSpacing * (infoLines.size() - 1);
    }

    float panelWidth = maxWidth + padding * 2;
    float panelHeight = totalHeight + padding * 2;

    Vec4<float> panelRect(hudPos.x, hudPos.y, hudPos.x + panelWidth, hudPos.y + panelHeight);

    Vec4<float> shadowRect(panelRect.x + 2, panelRect.y + 2, panelRect.z + 2, panelRect.w + 2);
    ImGuiUtils::fillRectangle(shadowRect, UIColor(0, 0, 0, 40), cornerRadius);

    UIColor bgColor = UIColor(20, 20, 25, 140);
    ImGuiUtils::fillRectangle(panelRect, bgColor, cornerRadius);

    UIColor themeColor = getColor();
    UIColor borderColor = UIColor(themeColor.r, themeColor.g, themeColor.b, 80);
    ImGuiUtils::drawRectangle(panelRect, borderColor, 1.0f, cornerRadius);

    float currentY = hudPos.y + padding;
    UIColor textColor = UIColor(240, 240, 245, 255);

    for(const auto& line : infoLines) {
        ImGuiUtils::drawText(Vec2<float>(hudPos.x + padding, currentY), line, textColor, textSize);
        currentY += textHeight + lineSpacing;
    }
}


void Interface::renderPotionEffects(const Vec2<float>& windowsSize, Actor* localPlayer,
                                    ImDrawList* drawlist) {
    std::string screenName = GI::getClientInstance()->getScreenName();
    if(screenName != "hud_screen")
        return;

    std::vector<EffectInfo> activeEffects;
    static std::unordered_map<int, int> effectTotalDurations;
    static std::unordered_map<int, float> effectAnimations;  // 保留仅用于防止瞬显瞬消

    float dt = ImGui::GetIO().DeltaTime;

    for(uint32_t effectId = 1; effectId < 37; effectId++) {
        MobEffect* mEffect = MobEffect::getById(effectId);
        if(mEffect == nullptr)
            continue;

        MobEffectInstance* mEffectInstance = localPlayer->hasEffect(mEffect);
        if(mEffectInstance == nullptr) {
            effectTotalDurations.erase(effectId);
            if(effectAnimations.count(effectId)) {
                effectAnimations[effectId] -= dt * 5.0f;  // 快速消失
                if(effectAnimations[effectId] <= 0.0f)
                    effectAnimations.erase(effectId);
            }
            continue;
        }

        if(!effectAnimations.count(effectId))
            effectAnimations[effectId] = 0.0f;
        if(effectAnimations[effectId] < 1.0f) {
            effectAnimations[effectId] += dt * 5.0f;  // 快速出现
            if(effectAnimations[effectId] > 1.0f)
                effectAnimations[effectId] = 1.0f;
        }

        EffectInfo info;
        info.name = getEffectName(effectId);
        info.level = mEffectInstance->mAmplifier + 1;
        info.duration = mEffectInstance->mDuration;
        info.timeLeft = getEffectTimeLeftStr(mEffectInstance);
        info.effectId = effectId;
        activeEffects.push_back(info);
    }

    if(activeEffects.empty() && effectAnimations.empty())
        return;

    std::sort(activeEffects.begin(), activeEffects.end(),
              [](const EffectInfo& a, const EffectInfo& b) { return a.duration < b.duration; });

    static Editor* editor = ModuleManager::getModule<Editor>();
    if(!editor)
        return;

    Vec2<float> pos = editor->getWidgetPosition("potion_effects");
    float scale = editor->getWidgetSize("potion_effects");

    float iconSize = 18.0f * scale;    // 图标大小
    float padding = 5.0f * scale;      // 紧凑的边距
    float itemSpacing = 2.0f * scale;  // 列表间距
    float textSize = 1.0f * scale;     // 字体大小

    float textHeight = ImGuiUtils::getTextHeight(textSize);
    float itemHeight = std::max(iconSize, textHeight * 2) + (padding * 2);

    float currentY = pos.y;

    for(const auto& effect : activeEffects) {
        float alpha =
            effectAnimations.count(effect.effectId) ? effectAnimations[effect.effectId] : 1.0f;
        if(alpha < 0.01f)
            continue;

        int globalAlpha = (int)(255 * alpha);

        std::string nameStr = effect.name;
        if(effect.level > 1)
            nameStr += " " + getRomanNumeral(effect.level);

        std::string timeStr = effect.timeLeft;

        float nameWidth = ImGuiUtils::getTextWidth(nameStr, textSize);
        float timeWidth = ImGuiUtils::getTextWidth(timeStr, textSize);
        float textBlockWidth = std::max(nameWidth, timeWidth);

        float itemWidth = padding + iconSize + padding + textBlockWidth + padding;

        Vec4<float> rect(pos.x, currentY, pos.x + itemWidth, currentY + itemHeight);

        UIColor bgColor = UIColor(30, 30, 30, (int)(180 * alpha));
        ImGuiUtils::fillRectangle(rect, bgColor, 2.0f * scale);  // 2.0f 圆角仅为了不割手

        UIColor effectColor = getEffectColor(effect.effectId);
        effectColor.a = globalAlpha;

        Vec4<float> iconRect(rect.x + padding, rect.y + (itemHeight - iconSize) / 2.0f,
                             rect.x + padding + iconSize,
                             rect.y + (itemHeight - iconSize) / 2.0f + iconSize);
        ImGuiUtils::fillRectangle(iconRect, effectColor, 0.0f);  // 0.0f = 直角，像素风

        float textX = rect.x + padding + iconSize + padding;
        float textStartY = rect.y + padding;

        ImGuiUtils::drawText(Vec2<float>(textX, textStartY), nameStr,
                             UIColor(255, 255, 255, globalAlpha), textSize);

        UIColor timeColor = (effect.duration < 200) ? UIColor(255, 80, 80, globalAlpha)
                                                    : UIColor(170, 170, 170, globalAlpha);

        ImGuiUtils::drawText(Vec2<float>(textX, textStartY + textHeight), timeStr, timeColor,
                             textSize);

        currentY += (itemHeight + itemSpacing);
    }
}
void Interface::renderKeystrokes(ImDrawList* drawlist) {
    std::string screenName = GI::getClientInstance()->getScreenName();
    if(screenName != "hud_screen")
        return;

    auto localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    static Editor* editor = ModuleManager::getModule<Editor>();
    if(!editor)
        return;

    Vec2<float> pos = editor->getWidgetPosition("keystrokes");
    float globalScale = editor->getWidgetSize("keystrokes");

    const float baseSize = 36.0f * globalScale;
    const float spacing = 4.0f * globalScale;
    const float radius = 4.0f * globalScale;
    const float fontSize = 1.0f * globalScale;

    struct KeyData {
        std::string label;
        int vkCode;
        Vec2<float> gridPos;
        float widthMult;
        float pressAnim;  // 按压动画
    };

    static std::vector<KeyData> keys = {
        {"W", 'W', {1, 0}, 1.0f, 0.f},
        {"A", 'A', {0, 1}, 1.0f, 0.f},
        {"S", 'S', {1, 1}, 1.0f, 0.f},
        {"D", 'D', {2, 1}, 1.0f, 0.f},
        {"SPACE", VK_SPACE, {0, 2}, 3.0f + (spacing / baseSize) * 2.0f, 0.f}};

    float dt = ImGui::GetIO().DeltaTime;

    UIColor themeColor = getColor();
    UIColor bgIdle = UIColor(20, 20, 25, 140);
    UIColor bgPress = UIColor(themeColor.r, themeColor.g, themeColor.b, 160);
    UIColor borderIdle = UIColor(60, 60, 70, 80);
    UIColor borderPress = UIColor(themeColor.r, themeColor.g, themeColor.b, 200);
    UIColor textIdle = UIColor(200, 200, 210, 255);
    UIColor textPress = UIColor(255, 255, 255, 255);

    for(auto& key : keys) {
        bool isDown = GI::isKeyDown(key.vkCode);

        float target = isDown ? 1.0f : 0.0f;
        float speed = isDown ? 16.0f : 12.0f;
        key.pressAnim += (target - key.pressAnim) * speed * dt;

        float width = baseSize * key.widthMult;
        float height = baseSize;
        float x = pos.x + (key.gridPos.x * (baseSize + spacing));
        float y = pos.y + (key.gridPos.y * (baseSize + spacing));

        float scale = 1.0f - (key.pressAnim * 0.04f);
        float scaledW = width * scale;
        float scaledH = height * scale;
        float offsetX = (width - scaledW) / 2.0f;
        float offsetY = (height - scaledH) / 2.0f;

        Vec4<float> rect(x + offsetX, y + offsetY, x + offsetX + scaledW, y + offsetY + scaledH);

        Vec4<float> shadowRect(rect.x + 2, rect.y + 2, rect.z + 2, rect.w + 2);
        ImGuiUtils::fillRectangle(shadowRect, UIColor(0, 0, 0, 40), radius);

        UIColor currBg = lerpColor(bgIdle, bgPress, key.pressAnim);
        ImGuiUtils::fillRectangle(rect, currBg, radius);

        UIColor currBorder = lerpColor(borderIdle, borderPress, key.pressAnim);
        ImGuiUtils::drawRectangle(rect, currBorder, 1.0f, radius);

        std::string label = (key.label == "SPACE") ? "━━━" : key.label;
        float tW = ImGuiUtils::getTextWidth(label, fontSize);
        float tH = ImGuiUtils::getTextHeight(fontSize);

        float textX = rect.x + (scaledW - tW) / 2.0f;
        float textY = rect.y + (scaledH - tH) / 2.0f;

        UIColor currText = lerpColor(textIdle, textPress, key.pressAnim);
        ImGuiUtils::drawText(Vec2<float>(textX, textY), label, currText, fontSize);
    }
}

void Interface::renderHotbar() {
    if(!GI::getClientInstance())
        return;
    std::string screenName = GI::getClientInstance()->getScreenName();
    if(screenName != "hud_screen")
        return;
    auto col = getColor();
    col.a = 100;
    ImGuiUtils::fillRectangle(sHotbarRectPos, col, 5.0f);
}
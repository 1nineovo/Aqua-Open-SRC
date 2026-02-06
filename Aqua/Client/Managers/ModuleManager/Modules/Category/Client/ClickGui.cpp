#include "ClickGui.h"

#include <algorithm>
#include <cmath>

#include "../../../../../../Libs/json.hpp"
#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../Utils/TimerUtil.h"
#include "../../../../../Client.h"
#include "../../../../../Managers/ModuleManager/ModuleManager.h"
#include "../Client/CustomFont.h"
#include <ImGuiUtils.h>

ClickGui::ClickGui()
    : Module("ClickGui", "NULL", Category::CLIENT, VK_INSERT) {
}

ClickGui::~ClickGui() {
    for(ClickWindow* window : windowList) {
        delete window;
    }
}

void ClickGui::onEnable() {
    GI::getClientInstance()->releaseVMouse();
    for(auto& module : ModuleManager::moduleList) {
        animProgress[module] = 0.0f;
    }
}

void ClickGui::onDisable() {
    GI::getClientInstance()->grabVMouse();
    isSearching = false;
    searchingModule.clear();
    draggingSliderSettingPtr = nullptr;
    capturingKbSettingPtr = nullptr;

    isLeftClickDown = false;
    isRightClickDown = false;
    isHoldingLeftClick = false;
    isHoldingRightClick = false;
}

void getModuleListByCategory(Category category, std::vector<Module*>& modList) {
    for(Module* mod : ModuleManager::moduleList) {
        if(mod->getCategory() == category) {
            modList.push_back(mod);
        }
    }
}

void ClickGui::onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown) {
    if(mouseButton == 1) {
        isLeftClickDown = isDown;
        isHoldingLeftClick = isDown;
    } else if(mouseButton == 2) {
        isRightClickDown = isDown;
        isHoldingRightClick = isDown;
    }
    switch(mouseButton) { 
    case 4:
            float moveVec = (isDown < 0) ? -15.f : 15.f;
            for(auto& window : windowList) {
                window->pos.y += moveVec;
            }
            break;
    }
}

void ClickGui::onKeyUpdate(int key, bool isDown) {
    if(key == VK_SHIFT)
        isShiftDown = isDown;
    if(key == VK_BACK)
        isBackKeyDown = isDown;
    if(!isEnabled()) {
        if(key == getKeybind() && isDown) {
            setEnabled(true);
        }
    } else {
        if(isDown) {
            if(editingTextSettingPtr != nullptr) {
                if(key == VK_ESCAPE) {
                    editingTextSettingPtr = nullptr;
                    textEditBuffer = "";
                    return;
                } else if(key == VK_RETURN) {
                    *editingTextSettingPtr->value = textEditBuffer;
                    editingTextSettingPtr = nullptr;
                    textEditBuffer = "";
                    return;
                } else if(key == VK_BACK) {
                    if(!textEditBuffer.empty()) {
                        textEditBuffer.pop_back();
                    }
                    return;
                } else {
                    char c = 0;
                    if(key >= 'A' && key <= 'Z') {
                        c = isShiftDown ? (char)key : (char)(key + 32);
                    } else if(key >= '0' && key <= '9') {
                        if(isShiftDown) {
                            const char shiftNums[] = ")!@#$%^&*(";
                            c = shiftNums[key - '0'];
                        } else {
                            c = (char)key;
                        }
                    } else if(key == VK_SPACE) {
                        c = ' ';
                    } else if(key == VK_OEM_MINUS) {
                        c = isShiftDown ? '_' : '-';
                    } else if(key == VK_OEM_PLUS) {
                        c = isShiftDown ? '+' : '=';
                    } else if(key == VK_OEM_4) {
                        c = isShiftDown ? '{' : '[';
                    } else if(key == VK_OEM_6) {
                        c = isShiftDown ? '}' : ']';
                    } else if(key == VK_OEM_1) {
                        c = isShiftDown ? ':' : ';';
                    } else if(key == VK_OEM_7) {
                        c = isShiftDown ? '"' : '\'';
                    } else if(key == VK_OEM_COMMA) {
                        c = isShiftDown ? '<' : ',';
                    } else if(key == VK_OEM_PERIOD) {
                        c = isShiftDown ? '>' : '.';
                    } else if(key == VK_OEM_2) {
                        c = isShiftDown ? '?' : '/';
                    }
                    
                    if(c != 0 && textEditBuffer.length() < (size_t)editingTextSettingPtr->maxLength) {
                        textEditBuffer += c;
                    }
                    return;
                }
            }
            
            if(key < 192) {
                if(capturingKbSettingPtr != nullptr) {
                    if(key != VK_ESCAPE)
                        *capturingKbSettingPtr->value = key;
                    capturingKbSettingPtr = nullptr;
                    return;
                }
            }
            if(key == getKeybind() || key == VK_ESCAPE) {
                setEnabled(false);
            }
            if(isSearching) {
                static auto isValid = [](char c) -> bool {
                    if(c >= '0' && c <= ' 9')
                        return true;
                    if(c >= 'a' && c <= 'z')
                        return true;
                    if(c >= 'A' && c <= 'Z')
                        return true;
                    return false;
                };
                if(key == VK_BACK && !searchingModule.empty())
                    searchingModule.pop_back();
                else if(key == ' ')
                    searchingModule += ' ';
                else if(isValid((char)key)) {
                    if(isShiftDown)
                        searchingModule += (char)key;
                    else
                        searchingModule += std::tolower(key);
                }
            }
        }
    }
}

void ClickGui::init() {
    setEnabled(false);

    Vec2<float> startPos = Vec2<float>(25.f, 25.f);
    this->windowList.emplace_back(new ClickWindow(startPos, Category::COMBAT));
    startPos.x += 220.f;
    this->windowList.emplace_back(new ClickWindow(startPos, Category::MISC));
    startPos.x += 220.f;
    this->windowList.emplace_back(new ClickWindow(startPos, Category::RENDER));
    startPos.x += 220.f;
    this->windowList.emplace_back(new ClickWindow(startPos, Category::MOVEMENT));
    startPos.x += 220.f;
    this->windowList.emplace_back(new ClickWindow(startPos, Category::PLAYER));
    startPos.x += 220.f;
    this->windowList.emplace_back(new ClickWindow(startPos, Category::CLIENT));
}

void ClickGui::render(ImDrawList* drawlist) {
    if(!initClickGui) {
        ClickGui::init();
        initClickGui = true;
    }
    if(!this->isEnabled()) {
        openDuration = 0.f;
        return;
    }
    if(GI::canUseMoveKeys())
        GI::getClientInstance()->releaseVMouse();

    static Interface* colorsMod = ModuleManager::getModule<Interface>();
    ImGuiIO& io = ImGui::GetIO();

    static const float windowWidth = 180.f;
    const float textHeight = ImGuiUtils::getTextHeight(textSize);
    static const float textPadding = 4.f;
    float roundValue = 10.0f;

    static const UIColor whiteColor(255, 255, 255, 255);
    static const UIColor grayColor(170, 170, 170, 255);
    static const UIColor darkGrayColor(40, 40, 40, 255);
    static const UIColor settingBgColor(15, 15, 15, 255);

    float targetDuration = 1.f;
    openDuration = Math::lerp(openDuration, targetDuration, io.DeltaTime);

    if(openDuration > 0.99f)
        openDuration = 1.f;

    UIColor accentColor(colorsMod->getColor());

    std::string tooltipString = "NULL";
    if(isLeftClickDown || isRightClickDown)
        isSearching = false;

    {
        UIColor dimColor(0, 0, 0, (int)(120 * openDuration));
        ImGuiUtils::fillRectangle(Vec4<float>(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y),
                                  dimColor);

        static Vec2<float> oldMousePos = mousePos;
        mouseDelta = mousePos.sub(oldMousePos);
        oldMousePos = mousePos;

        for(ClickWindow* window : windowList) {
            if(window->pos.x > io.DisplaySize.x)
                window->pos.x = io.DisplaySize.x - 200.f;
            if(window->pos.y > io.DisplaySize.y)
                window->pos.y = io.DisplaySize.y - 200.f;

            if(window->isDragging) {
                window->pos.x += mousePos.x - startDragPos.x;
                window->pos.y += mousePos.y - startDragPos.y;
                startDragPos = mousePos;
                if(!isHoldingLeftClick)
                    window->isDragging = false;
            }

            std::vector<Module*> visibleModules;
            if(window->extended) {
                for(Module* mod : window->modList) {
                    if(!searchingModule.empty()) {
                        std::string lowerSearch = searchingModule;
                        std::string lowerModule = mod->getModuleName();
                        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(),
                                       ::tolower);
                        std::transform(lowerModule.begin(), lowerModule.end(), lowerModule.begin(),
                                       ::tolower);
                        if(lowerModule.find(lowerSearch) == std::string::npos)
                            continue;
                    }
                    visibleModules.push_back(mod);
                }
            }

            float headerH = textHeight + textPadding * 3.f;
            Vec4<float> headerRect = Vec4<float>(
                window->pos.x, window->pos.y, window->pos.x + windowWidth, window->pos.y + headerH);

            ImDrawFlags headerFlags =
                window->extended ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersAll;

            drawlist->AddRectFilled(ImVec2(headerRect.x, headerRect.y),
                                    ImVec2(headerRect.z, headerRect.w), IM_COL32(35, 35, 35, 255),
                                    roundValue, headerFlags);

            drawlist->AddRect(ImVec2(headerRect.x, headerRect.y),
                              ImVec2(headerRect.z, headerRect.w), IM_COL32(60, 60, 60, 255),
                              roundValue, headerFlags,
                              1.5f
            );

            float titleWidth = ImGuiUtils::getTextWidth(window->windowName, textSize * 1.1f);
            Vec2<float> titlePos = Vec2<float>(headerRect.x + (windowWidth - titleWidth) / 2.f,
                                               headerRect.y + textPadding * 1.5f);
            ImGuiUtils::drawText(titlePos, window->windowName, whiteColor, textSize * 1.1f, true);

            if(headerRect.contains(mousePos)) {
                if(isLeftClickDown && !window->isDragging) {
                    window->isDragging = true;
                    startDragPos = mousePos;
                    isLeftClickDown = false;
                } else if(isRightClickDown) {
                    window->extended = !window->extended;
                    isRightClickDown = false;
                }
            }

            float xStart = headerRect.x;
            float xEnd = headerRect.z;
            float yOffset = headerRect.w;

            if(window->extended && !visibleModules.empty()) {
                int modIndex = 0;
                for(size_t i = 0; i < visibleModules.size(); i++) {
                    Module* mod = visibleModules[i];
                    bool isLastModule =
                        (i == visibleModules.size() - 1);

                    UIColor modColor = colorsMod->getColor(-modIndex);
                    modIndex += colorsMod->seperation;

                    float modH = textHeight + textPadding * 3.f;
                    Vec4<float> modRect = {xStart, yOffset, xEnd, yOffset + modH};

                    float& progress = animProgress[mod];
                    float targetProgress = mod->isEnabled() ? 1.0f : 0.0f;
                    progress = Math::lerp(progress, targetProgress, io.DeltaTime * 25.f);

                    bool isHovered = modRect.contains(mousePos);

                    float modRound = 0.0f;
                    ImDrawFlags modFlags = 0;
                    if(isLastModule && !mod->extended) {
                        modRound = roundValue;
                        modFlags = ImDrawFlags_RoundCornersBottom;
                    }

                    ImU32 cellBg =
                        isHovered ? IM_COL32(35, 35, 35, 250) : IM_COL32(20, 20, 20, 250);
                    drawlist->AddRectFilled(ImVec2(modRect.x, modRect.y),
                                            ImVec2(modRect.z, modRect.w), cellBg, modRound,
                                            modFlags);

                    if(progress > 0.01f) {
                        int alpha = (int)(150 * progress);
                        drawlist->AddRectFilled(ImVec2(modRect.x, modRect.y),
                                                ImVec2(modRect.z, modRect.w),
                                                modColor.toImColor(alpha), modRound, modFlags);
                    }

                    if(progress > 0.01f) {
                        float capsuleWidth = 2.0f;  
                        float capsuleMargin = 3.0f;  
                        float heightRatio =
                            0.6f; 

                        float modH = modRect.w - modRect.y;
                        float targetH = modH * heightRatio;

                        float currentH = targetH * progress;
                        float centerY = modRect.y + modH / 2.f;

                        drawlist->AddRectFilled(
                            ImVec2(modRect.x + capsuleMargin, centerY - currentH / 2.f),
                            ImVec2(modRect.x + capsuleMargin + capsuleWidth,
                                   centerY + currentH / 2.f),
                            modColor.toImColor((int)(255 * progress)),
                            capsuleWidth / 2.0f
                        );
                    }

                    Vec2<float> textPos = {modRect.x + textPadding * 3.f + (5.f * progress),
                                           modRect.y + textPadding * 1.5f};
                    UIColor txtColor = (mod->isEnabled() || isHovered) ? whiteColor : grayColor;
                    ImGuiUtils::drawText(textPos, mod->getModuleName(), txtColor, textSize, true);

                    if(isHovered) {
                        tooltipString = mod->getDescription();
                        if(isLeftClickDown) {
                            mod->toggle();
                            isLeftClickDown = false;
                        } else if(isRightClickDown) {
                            mod->extended = !mod->extended;
                            isRightClickDown = false;
                        }
                    }

                    yOffset += modH;

                    if(mod->extended) {
                        std::vector<Setting*> visibleSettings;
                        for(auto& s : mod->getSettingList()) {
                                visibleSettings.push_back(s);
                        }

                        for(size_t j = 0; j < visibleSettings.size(); j++) {
                            Setting* setting = visibleSettings[j];
                            bool isLastSetting = (j == visibleSettings.size() - 1);

                            float itemHeight = textHeight + textPadding * 2.5f;
                            if(setting->type == SettingType::SLIDER_S)
                                itemHeight += 12.0f;
                            if(setting->type == SettingType::COLOR_S) {
                                ColorSetting* cs = static_cast<ColorSetting*>(setting);
                                if(cs->extended)
                                    itemHeight += 160.0f;
                            }

                            Vec4<float> setRect = {xStart, yOffset, xEnd, yOffset + itemHeight};

                            float setRound = 0.0f;
                            ImDrawFlags setFlags = 0;
                            if(isLastModule && isLastSetting) {
                                setRound = roundValue;
                                setFlags = ImDrawFlags_RoundCornersBottom;
                            }

                            drawlist->AddRectFilled(ImVec2(setRect.x, setRect.y),
                                                    ImVec2(setRect.z, setRect.w),
                                                    settingBgColor.toImColor(), setRound, setFlags);

                            Vec4<float> contentRect = {setRect.x + 15.f, setRect.y, setRect.z - 5.f,
                                                       setRect.w};
                            Vec2<float> sTextPos = {contentRect.x, contentRect.y + textPadding};

                            if(contentRect.contains(mousePos))
                                tooltipString = setting->description;

                            UIColor sColor = colorsMod->getColor(-modIndex);
                            modIndex += 15;


                            switch(setting->type) {
                                case(SettingType::BOOL_S): {
                                    BoolSetting* bs = static_cast<BoolSetting*>(setting);
                                    bool val = *bs->value;
                                    ImGuiUtils::drawText(sTextPos, setting->name, whiteColor,
                                                         textSize, true);

                                    float switchW = 30.f;
                                    float switchH = 16.f;
                                    float midY = setRect.y + (itemHeight - switchH) / 2.f;
                                    Vec4<float> switchRect = {contentRect.z - switchW, midY,
                                                              contentRect.z, midY + switchH};

                                    UIColor toggleBg = val ? sColor : UIColor(60, 60, 60, 255);
                                    ImGuiUtils::fillRectangle(switchRect, toggleBg, 8.f);

                                    float circleX = val ? (switchRect.z - switchH / 2.f)
                                                        : (switchRect.x + switchH / 2.f);
                                    drawlist->AddCircleFilled(
                                        ImVec2(circleX, switchRect.y + switchH / 2.f),
                                        switchH / 2.f - 2.f, IM_COL32(255, 255, 255, 255));

                                    if(contentRect.contains(mousePos) && isLeftClickDown) {
                                        *bs->value = !val;
                                        isLeftClickDown = false;
                                    }
                                    break;
                                }
                                case(SettingType::SLIDER_S): {
                                    SliderSettingBase* ssBase =
                                        static_cast<SliderSettingBase*>(setting);
                                    if(!ssBase)
                                        break;

                                    ImGuiUtils::drawText(sTextPos, setting->name, whiteColor,
                                                         textSize, true);

                                    std::string valStr;
                                    float percent = 0.f;
                                    if(ssBase->valueType == ValueType::INT_T) {
                                        SliderSetting<int>* si =
                                            static_cast<SliderSetting<int>*>(ssBase);
                                        if(si && si->valuePtr) {
                                            valStr = std::to_string(*si->valuePtr);
                                            percent = (float)(*si->valuePtr - si->minValue) /
                                                      (float)(si->maxValue - si->minValue);
                                        }
                                    } else {
                                        SliderSetting<float>* sf =
                                            static_cast<SliderSetting<float>*>(ssBase);
                                        if(sf && sf->valuePtr) {
                                            char buf[32];
                                            sprintf_s(buf, "%.2f", *sf->valuePtr);
                                            valStr = buf;
                                            percent = (*sf->valuePtr - sf->minValue) /
                                                      (sf->maxValue - sf->minValue);
                                        }
                                    }
                                    percent = std::clamp(percent, 0.f, 1.f);

                                    float valWidth = ImGuiUtils::getTextWidth(valStr, textSize);
                                    ImGuiUtils::drawText(
                                        Vec2<float>(contentRect.z - valWidth, sTextPos.y), valStr,
                                        grayColor, textSize, true);

                                    float sliderTop = sTextPos.y + textHeight + 4.f;
                                    Vec4<float> sliderRect = {contentRect.x, sliderTop,
                                                              contentRect.z, sliderTop + 4.f};

                                    if(contentRect.contains(mousePos) && isLeftClickDown) {
                                        setting->isDragging = true;
                                        draggingSliderSettingPtr = ssBase;
                                        isLeftClickDown = false;
                                    }
                                    if(setting->isDragging) {
                                        if(!isHoldingLeftClick) {
                                            setting->isDragging = false;
                                            draggingSliderSettingPtr = nullptr;
                                        } else {
                                            float newP =
                                                std::clamp((mousePos.x - sliderRect.x) /
                                                               (sliderRect.z - sliderRect.x),
                                                           0.f, 1.f);
                                            percent = newP;
                                            if(ssBase->valueType == ValueType::INT_T) {
                                                SliderSetting<int>* si =
                                                    static_cast<SliderSetting<int>*>(ssBase);
                                                if(si)
                                                    *si->valuePtr =
                                                        (int)(si->minValue +
                                                              (si->maxValue - si->minValue) * newP);
                                            } else {
                                                SliderSetting<float>* sf =
                                                    static_cast<SliderSetting<float>*>(ssBase);
                                                if(sf)
                                                    *sf->valuePtr =
                                                        sf->minValue +
                                                        (sf->maxValue - sf->minValue) * newP;
                                            }
                                        }
                                    }

                                    ImGuiUtils::fillRectangle(sliderRect, UIColor(60, 60, 60, 255),
                                                              2.f);
                                    Vec4<float> fillRect = {
                                        sliderRect.x, sliderRect.y,
                                        sliderRect.x + (sliderRect.z - sliderRect.x) * percent,
                                        sliderRect.w};
                                    ImGuiUtils::fillRectangle(fillRect, sColor, 2.f);
                                    drawlist->AddCircleFilled(
                                        ImVec2(fillRect.z, (sliderRect.y + sliderRect.w) / 2.f),
                                        5.f, IM_COL32(255, 255, 255, 255));
                                    break;
                                }
                                case(SettingType::ENUM_S): {
                                    EnumSetting* es = static_cast<EnumSetting*>(setting);
                                    ImGuiUtils::drawText(sTextPos, setting->name + ":", whiteColor,
                                                         textSize, true);

                                    if(es->value && *es->value >= 0 &&
                                       *es->value < es->enumList.size()) {
                                        std::string modeName = es->enumList[*es->value];
                                        float modeW = ImGuiUtils::getTextWidth(modeName, textSize);
                                        ImGuiUtils::drawText(
                                            Vec2<float>(contentRect.z - modeW, sTextPos.y),
                                            modeName, grayColor, textSize, true);
                                    }

                                    if(contentRect.contains(mousePos)) {
                                        if(isLeftClickDown) {
                                            (*es->value)++;
                                            if(*es->value >= es->enumList.size())
                                                *es->value = 0;
                                            isLeftClickDown = false;
                                        } else if(isRightClickDown) {
                                            (*es->value)--;
                                            if(*es->value < 0)
                                                *es->value = (int)es->enumList.size() - 1;
                                            isRightClickDown = false;
                                        }
                                    }
                                    break;
                                }
                                case(SettingType::COLOR_S): {
                                    ColorSetting* cs = static_cast<ColorSetting*>(setting);

                                    ImGuiUtils::drawText(sTextPos, setting->name + ":", whiteColor,
                                                         textSize, true);

                                    float headerH = textHeight + textPadding * 2.5f;
                                    Vec4<float> previewRect = {
                                        contentRect.z - 25.f, setRect.y + (headerH - 12.f) / 2.f,
                                        contentRect.z - 5.f, setRect.y + (headerH + 12.f) / 2.f};

                                    ImGuiUtils::fillRectangle(previewRect, *cs->colorPtr, 4.f);
                                    drawlist->AddRect(ImVec2(previewRect.x, previewRect.y),
                                                      ImVec2(previewRect.z, previewRect.w),
                                                      IM_COL32(80, 80, 80, 255), 4.f);

                                    if(previewRect.contains(mousePos) && isLeftClickDown) {
                                        cs->extended = !cs->extended;
                                        isLeftClickDown = false;
                                    }

                                  if(cs->extended) {
                                        float currentY = setRect.y + headerH + 6.f;

                                        float toggleH = 22.f;
                                        Vec4<float> toggleRowRect = {contentRect.x, currentY,
                                                                     contentRect.z,
                                                                     currentY + toggleH};

                                        float centerY = toggleRowRect.y + toggleH / 2.f;

                                        float textY = centerY - textHeight / 2.f;
                                        ImGuiUtils::drawText(Vec2<float>(toggleRowRect.x, textY),
                                                             "Theme", whiteColor, textSize);

                                        float switchW = 30.f;
                                        float switchH = 14.f;
                                        float switchY = centerY - switchH / 2.f;

                                        Vec4<float> sRect = {toggleRowRect.z - switchW, switchY,
                                                             toggleRowRect.z, switchY + switchH};

                                        bool themeActive = cs->theme;

                                        UIColor toggleBg =
                                            themeActive ? accentColor : UIColor(60, 60, 60, 255);
                                        ImGuiUtils::fillRectangle(sRect, toggleBg, 8.f);

                                        float circleSize = switchH - 4.f;
                                        float circleX = themeActive ? (sRect.z - switchH / 2.f)
                                                                    : (sRect.x + switchH / 2.f);
                                        drawlist->AddCircleFilled(
                                            ImVec2(circleX, switchY + switchH / 2.f),
                                            circleSize / 2.f, IM_COL32(255, 255, 255, 255));

                                        if(toggleRowRect.contains(mousePos) && isLeftClickDown) {
                                            cs->theme = !cs->theme;
                                            isLeftClickDown = false;
                                        }

                                        currentY += toggleH + 8.f;

                                        float pickerHeight = 100.f;

                                        Vec4<float> pickerArea = {contentRect.x, currentY,
                                                                  contentRect.z,
                                                                  currentY + pickerHeight};

                                        float barWidth = 12.f;
                                        float spacing = 6.f;
                                        Vec4<float> alphaRect = {contentRect.z - barWidth, currentY,
                                                                 contentRect.z,
                                                                 currentY + pickerHeight};
                                        Vec4<float> hueRect = {alphaRect.x - spacing - barWidth,
                                                               currentY, alphaRect.x - spacing,
                                                               currentY + pickerHeight};
                                        Vec4<float> svRect = {contentRect.x, currentY,
                                                              hueRect.x - spacing,
                                                              currentY + pickerHeight};

                                        float r = cs->colorPtr->r / 255.f;
                                        float g = cs->colorPtr->g / 255.f;
                                        float b = cs->colorPtr->b / 255.f;
                                        float h, s, v;
                                        ImGui::ColorConvertRGBtoHSV(r, g, b, h, s, v);

                                        static ColorSetting* activeCs = nullptr;
                                        static int dragMode = 0;
                                        if(!isHoldingLeftClick) {
                                            activeCs = nullptr;
                                            dragMode = 0;
                                        }

                                        if(isLeftClickDown) {
                                            if(svRect.contains(mousePos)) {
                                                activeCs = cs;
                                                dragMode = 1;
                                                isLeftClickDown = false;
                                            } else if(hueRect.contains(mousePos)) {
                                                activeCs = cs;
                                                dragMode = 2;
                                                isLeftClickDown = false;
                                            } else if(alphaRect.contains(mousePos)) {
                                                activeCs = cs;
                                                dragMode = 3;
                                                isLeftClickDown = false;
                                            }
                                        }

                                        if(activeCs == cs && isHoldingLeftClick) {
                                            if(dragMode == 1) {
                                                s = std::clamp(
                                                    (mousePos.x - svRect.x) / (svRect.z - svRect.x),
                                                    0.f, 1.f);
                                                v = std::clamp(1.f - (mousePos.y - svRect.y) /
                                                                         (svRect.w - svRect.y),
                                                               0.f, 1.f);
                                            } else if(dragMode == 2) {
                                                h = std::clamp((mousePos.y - hueRect.y) /
                                                                   (hueRect.w - hueRect.y),
                                                               0.f, 1.f);
                                            } else if(dragMode == 3) {
                                                float newA =
                                                    1.f -
                                                    std::clamp((mousePos.y - alphaRect.y) /
                                                                   (alphaRect.w - alphaRect.y),
                                                               0.f, 1.f);
                                                cs->colorPtr->a = (int)(newA * 255.f);
                                            }

                                            if(dragMode != 3) {
                                                ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
                                                cs->colorPtr->r = (int)(r * 255.f);
                                                cs->colorPtr->g = (int)(g * 255.f);
                                                cs->colorPtr->b = (int)(b * 255.f);
                                            }
                                        }


                                        float hR, hG, hB;
                                        ImGui::ColorConvertHSVtoRGB(h, 1.f, 1.f, hR, hG, hB);
                                        ImU32 hueC = IM_COL32((int)(hR * 255), (int)(hG * 255),
                                                              (int)(hB * 255), 255);
                                        drawlist->AddRectFilledMultiColor(
                                            ImVec2(svRect.x, svRect.y), ImVec2(svRect.z, svRect.w),
                                            IM_COL32(255, 255, 255, 255), hueC, hueC,
                                            IM_COL32(255, 255, 255, 255));
                                        drawlist->AddRectFilledMultiColor(
                                            ImVec2(svRect.x, svRect.y), ImVec2(svRect.z, svRect.w),
                                            0, 0, IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 255));

                                        float dotX = svRect.x + s * (svRect.z - svRect.x);
                                        float dotY = svRect.y + (1.f - v) * (svRect.w - svRect.y);
                                        drawlist->AddCircle(ImVec2(dotX, dotY), 3.f,
                                                            IM_COL32(255, 255, 255, 255));
                                        drawlist->AddCircle(ImVec2(dotX, dotY), 4.f,
                                                            IM_COL32(0, 0, 0, 255));

                                        static const ImU32 hC[] = {
                                            IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255),
                                            IM_COL32(0, 255, 0, 255), IM_COL32(0, 255, 255, 255),
                                            IM_COL32(0, 0, 255, 255), IM_COL32(255, 0, 255, 255),
                                            IM_COL32(255, 0, 0, 255)};
                                        for(int i = 0; i < 6; i++) {
                                            drawlist->AddRectFilledMultiColor(
                                                ImVec2(hueRect.x,
                                                       hueRect.y +
                                                           (i * (hueRect.w - hueRect.y) / 6.f)),
                                                ImVec2(hueRect.z,
                                                       hueRect.y + ((i + 1) *
                                                                    (hueRect.w - hueRect.y) / 6.f)),
                                                hC[i], hC[i], hC[i + 1], hC[i + 1]);
                                        }
                                        float hy = hueRect.y + h * (hueRect.w - hueRect.y);
                                        ImGuiUtils::fillRectangle(
                                            Vec4<float>(hueRect.x, hy - 1, hueRect.z, hy + 1),
                                            whiteColor);

                                        drawlist->AddRectFilled(ImVec2(alphaRect.x, alphaRect.y),
                                                                ImVec2(alphaRect.z, alphaRect.w),
                                                                IM_COL32(50, 50, 50, 255));

                                        ImU32 cT = IM_COL32((int)(r * 255), (int)(g * 255),
                                                            (int)(b * 255), 255);
                                        ImU32 cB = IM_COL32((int)(r * 255), (int)(g * 255),
                                                            (int)(b * 255), 0);
                                        drawlist->AddRectFilledMultiColor(
                                            ImVec2(alphaRect.x, alphaRect.y),
                                            ImVec2(alphaRect.z, alphaRect.w), cT, cT, cB, cB);

                                        float ay = alphaRect.y + (1.f - cs->colorPtr->a / 255.f) *
                                                                     (alphaRect.w - alphaRect.y);
                                        ImGuiUtils::fillRectangle(
                                            Vec4<float>(alphaRect.x, ay - 1, alphaRect.z, ay + 1),
                                            whiteColor);
                                    }
                                    break;
                                }
                                default: {
                                    ImGuiUtils::drawText(sTextPos, setting->name, whiteColor,
                                                         textSize, true);
                                    if(setting->type == SettingType::KEYBIND_S) {
                                        KeybindSetting* kb = static_cast<KeybindSetting*>(setting);
                                        std::string kn =
                                            (*kb->value == 0) ? "None" : KeyNames[*kb->value];
                                        if(setting == capturingKbSettingPtr)
                                            kn = "...";
                                        ImGuiUtils::drawText(
                                            Vec2<float>(contentRect.z -
                                                            ImGuiUtils::getTextWidth(kn, textSize),
                                                        sTextPos.y),
                                            kn, grayColor, textSize, true);
                                        if(contentRect.contains(mousePos)) {
                                            if(isLeftClickDown) {
                                                capturingKbSettingPtr =
                                                    (capturingKbSettingPtr == kb) ? nullptr : kb;
                                                isLeftClickDown = false;
                                            } else if(isRightClickDown) {
                                                *kb->value = 0;
                                                isRightClickDown = false;
                                            }
                                        }
                                    }
                                    break;
                                }
                                case(SettingType::TEXT_S): {
                                    TextSetting* ts = static_cast<TextSetting*>(setting);
                                    ImGuiUtils::drawText(sTextPos, setting->name + ":", whiteColor,
                                                         textSize, true);
                                    std::string displayText;
                                    bool isEditing = (editingTextSettingPtr == ts);
                                    
                                    if(isEditing) {
                                        displayText = textEditBuffer;
                                    } else {
                                        displayText = ts->value->empty() ? "Click to edit" : *ts->value;
                                    }

 
                                    float maxTextWidth = contentRect.z - contentRect.x - ImGuiUtils::getTextWidth(setting->name + ": ", textSize) - 10.f;
                                    while(ImGuiUtils::getTextWidth(displayText, textSize) > maxTextWidth && displayText.length() > 3) {
                                        displayText = displayText.substr(0, displayText.length() - 4) + "...";
                                    }

                                    UIColor textDisplayColor = isEditing ? accentColor : (ts->value->empty() ? UIColor(100, 100, 100, 255) : grayColor);
                                    float textDisplayWidth = ImGuiUtils::getTextWidth(displayText, textSize);
                                    ImGuiUtils::drawText(
                                        Vec2<float>(contentRect.z - textDisplayWidth, sTextPos.y),
                                        displayText, textDisplayColor, textSize, true);

                                    if(isEditing) {
                                        static float blink = 0.f;
                                        blink += io.DeltaTime * 2.f;
                                        if(blink > 1.f) blink = 0.f;
                                        
                                        if(blink < 0.5f) {
                                            float cursorX = contentRect.z - textDisplayWidth + ImGuiUtils::getTextWidth(textEditBuffer, textSize) + 2.f;
                                            float cursorY = sTextPos.y;
                                            float cursorH = textHeight;
                                            ImGuiUtils::fillRectangle(
                                                Vec4<float>(cursorX, cursorY, cursorX + 1.5f, cursorY + cursorH),
                                                accentColor);
                                        }
                                    }

                                    if(contentRect.contains(mousePos) && isLeftClickDown) {
                                        if(editingTextSettingPtr == ts) {

                                            *ts->value = textEditBuffer;
                                            editingTextSettingPtr = nullptr;
                                            textEditBuffer = "";
                                        } else {
 
                                            editingTextSettingPtr = ts;
                                            textEditBuffer = *ts->value;
                                        }
                                        isLeftClickDown = false;
                                    }
                                    break;
                                }
                            }
                            yOffset += itemHeight;
                        }
                    }
                }
            }
            if(window->extended) {
                drawlist->AddRect(
                    ImVec2(window->pos.x, window->pos.y + headerH - 1.f),
                    ImVec2(window->pos.x + windowWidth, yOffset), IM_COL32(60, 60, 60, 255),
                    roundValue, ImDrawFlags_RoundCornersBottom, 1.5f);
            }
        }

        static float focusAnim = 0.f;
        float targetFocus = isSearching ? 1.0f : 0.0f;
        focusAnim = Math::lerp(focusAnim, targetFocus, io.DeltaTime * 10.f);

        static const float baseSearchW = 350.f;
        static const float searchH = 38.f;

        float currentSearchW = baseSearchW + (60.f * focusAnim);

        float searchX = io.DisplaySize.x / 2.f - currentSearchW / 2.f;

        float searchY = io.DisplaySize.y - searchH - 50.f;

        Vec4<float> searchRect = {searchX, searchY, searchX + currentSearchW, searchY + searchH};

        int bgAlpha = (int)240;
        ImGuiUtils::fillRectangle(searchRect, UIColor(30, 30, 30, bgAlpha), searchH / 2.f);

        int borderAlpha = (int)(255 * openDuration);
        UIColor borderColor(60, 60, 60, borderAlpha);
        ImGuiUtils::drawRectangle(searchRect, borderColor, 1.5f + (0.5f * focusAnim),
                                  searchH / 2.f);

        float iconAlpha = 1.0f;
        UIColor iconColor(170, 170, 170, (int)(255 * iconAlpha * openDuration));

        float iconW = ImGuiUtils::getTextWidth("Search:", textSize);
        ImGuiUtils::drawText(
            Vec2<float>(searchX + 15.f, searchY + searchH / 2.f - textHeight / 2.f),
            "Search:", iconColor, textSize);

        float textStartX = searchX + 15.f + iconW + 10.f;
        Vec4<float> inputRect = {textStartX, searchRect.y, searchRect.z - 15.f, searchRect.w};

        if(searchRect.contains(mousePos) && isLeftClickDown) {
            if(isChoosingKeyBindPtr)
                *isChoosingKeyBindPtr = false;
            isSearching = true;
            isLeftClickDown = false;
        }

        static float holdTimes = 0.f;
        if(isBackKeyDown)
            holdTimes += io.DeltaTime;
        else
            holdTimes = 0.f;
        if(holdTimes > 0.5f && !searchingModule.empty() && isSearching)
            searchingModule.pop_back();

        Vec2<float> inputTextPos = {textStartX, searchY + searchH / 2.f - textHeight / 2.f};

        drawlist->PushClipRect(ImVec2(searchRect.x, searchRect.y),
                               ImVec2(searchRect.z, searchRect.w));

        if(!searchingModule.empty()) {
            ImGuiUtils::drawText(inputTextPos, searchingModule,
                                 UIColor(255, 255, 255, (int)(255 * openDuration)), textSize);
        } else if(!isSearching) {
            ImGuiUtils::drawText(inputTextPos, "Type to filter...",
                                 UIColor(120, 120, 120, (int)(255 * openDuration)), textSize);
        }

        if(isSearching) {
            static float blink = 0.f;
            blink += io.DeltaTime * 2.f;
            if(blink > 1.f)
                blink = 0.f;

            if(blink < 0.5f) {
                float txtW = ImGuiUtils::getTextWidth(searchingModule, textSize);
                float cursorX = inputTextPos.x + txtW + 2.f;
                float cursorH = 16.f;
                float cursorY = searchY + (searchH - cursorH) / 2.f;

                ImGuiUtils::fillRectangle(
                    Vec4<float>(cursorX, cursorY, cursorX + 1.5f, cursorY + cursorH),
                    UIColor(255, 255, 255, (int)(255 * openDuration)));
            }
        }
        drawlist->PopClipRect();

        lastKeyPress = -1;
    }
}
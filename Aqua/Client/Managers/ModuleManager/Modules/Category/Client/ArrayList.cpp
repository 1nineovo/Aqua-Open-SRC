#include "Arraylist.h"

#include <ImGuiUtils.h>

#include <algorithm>

#include "../../../ModuleManager.h"

Arraylist::Arraylist() : Module("Arraylist", "Display enabled modules", Category::CLIENT) {
    registerSetting(new SliderSetting<int>("Offset", "X Offset", &offset, 5, 0, 50));
    registerSetting(new SliderSetting<int>("Gap", "Gap Between Items", &gap, 2, 0, 10));
    registerSetting(
        new SliderSetting<float>("Rounding", "Corner Rounding", &rounding, 4.0f, 0.0f, 15.0f));
    registerSetting(
        new SliderSetting<float>("Opacity", "Background Opacity", &bgOpacity, 0.7f, 0.0f, 1.0f));
    registerSetting(new BoolSetting("Glow", "Glow Effect", &glow, true));
    registerSetting(new BoolSetting("Bottom", "Align to Bottom", &bottom, false));
}


bool sortByLength(Module* lhs, Module* rhs) {
    float textWidth1 = ImGuiUtils::getTextWidth(lhs->getModuleName());
    if(lhs->getModeText() != "NULL")
        textWidth1 += ImGuiUtils::getTextWidth(std::string(" " + lhs->getModeText()));
    float textWidth2 = ImGuiUtils::getTextWidth(rhs->getModuleName());
    if(rhs->getModeText() != "NULL")
        textWidth2 += ImGuiUtils::getTextWidth(std::string(" " + rhs->getModeText()));

    return (textWidth1 > textWidth2);
}

void Arraylist::onImGuiRender(ImDrawList* drawlist) {
    static ClickGui* guiMod = ModuleManager::getModule<ClickGui>();
    if(guiMod->isEnabled())
        return;

    static Interface* colorsMod = ModuleManager::getModule<Interface>();

    float textSize = 1.0f * size;
    float paddingX = 5.0f;
    float paddingY = 1.5f; 

    float textHeight = ImGuiUtils::getTextHeight(textSize);
    float moduleHeight = textHeight + (paddingY * 2.0f);

    float barWidth = 3.0f; 

    ImGuiIO& io = ImGui::GetIO();
    const float deltaTime = io.DeltaTime;
    float screenW = GI::getGuiData()->windowSizeReal.x;
    float screenH = GI::getGuiData()->windowSizeReal.y;

    float baseX = screenW - ((float)offset * 5);

    float currentY = bottom ? (screenH - (float)offset * 5) : ((float)offset * 5);

    static std::vector<Module*> moduleList = ModuleManager::moduleList;

    std::sort(moduleList.begin(), moduleList.end(), sortByLength);

    int visibleCount = 0;
    for(Module* m : moduleList)
        if(m->isEnabled() && m->isVisible())
            visibleCount++;

    float colorStep = 0.0f;

    for(Module* module : moduleList) {
        bool shouldRender = module->isEnabled() && module->isVisible();
        float targetAnim = shouldRender ? 1.0f : 0.0f;

        module->ArrayListDuration =
            Math::lerp(module->ArrayListDuration, targetAnim, deltaTime * 15.0f);

        if(module->ArrayListDuration < 0.01f)
            continue;

        std::string name = module->getModuleName();
        std::string mode = module->getModeText();

        float nameW = ImGuiUtils::getTextWidth(name, textSize);
        float modeW = (mode != "NULL") ? ImGuiUtils::getTextWidth(" " + mode, textSize) : 0.f;

        float contentWidth = nameW + modeW + (paddingX * 2.0f);
        float totalRectWidth = contentWidth + barWidth + 1.0f; 

        float xAnimOffset = (1.0f - module->ArrayListDuration) * (totalRectWidth + 10.0f);

        float xRight = baseX + xAnimOffset;
        float xLeft = xRight - totalRectWidth;

        float currentItemHeight = moduleHeight * module->ArrayListDuration;
        float currentItemGap = (float)gap * module->ArrayListDuration;

        float yTop, yBottom;

        if(bottom) {
            yBottom = currentY;
            yTop = currentY - currentItemHeight;

            currentY -= (currentItemHeight + currentItemGap);
        } else {
            yTop = currentY;
            yBottom = currentY + currentItemHeight;

            currentY += (currentItemHeight + currentItemGap);
        }

        ImVec2 rectMin(xLeft, yTop);
        ImVec2 rectMax(xRight, yBottom);

        float colorPos =
            colorStep * colorsMod->seperation; 
        colorStep += 1.0f;

        UIColor mainColor = colorsMod->getColor(-colorPos); 
        UIColor secColor = colorsMod->getColor(-(colorPos + 20.f));

        float alpha = module->ArrayListDuration; 

        if(this->glow) {
            ImU32 glowCol = mainColor.toImColor((int)(80 * alpha));
            drawlist->AddRectFilled(ImVec2(rectMin.x - 2.f, rectMin.y - 2.f),
                                    ImVec2(rectMax.x + 2.f, rectMax.y + 2.f), glowCol,
                                    rounding + 2.0f 
            );
        }

        int bgAlphaVal = (int)(255.0f * bgOpacity * alpha);
        ImU32 bgLeft = IM_COL32(0, 0, 0, (int)(20 * alpha));
        ImU32 bgRight = IM_COL32(0, 0, 0, bgAlphaVal);

        drawlist->AddRectFilledMultiColor(rectMin, rectMax, bgLeft, bgRight, bgRight, bgLeft);

        ImVec2 barMin(xRight - barWidth, yTop);
        ImVec2 barMax(xRight, yBottom);

        drawlist->AddRectFilledMultiColor(barMin, barMax, mainColor.toImColor((int)(255 * alpha)),
                                          mainColor.toImColor((int)(255 * alpha)),
                                          secColor.toImColor((int)(255 * alpha)),
                                          secColor.toImColor((int)(255 * alpha)));

        float textCenterY = yTop + (currentItemHeight - textHeight) / 2.0f;
        float textX = xLeft + paddingX;

        ImGuiUtils::drawText(Vec2<float>(textX + 1.f, textCenterY + 1.f), name,
                             UIColor(0, 0, 0, (int)(180 * alpha)), textSize);
        ImGuiUtils::drawText(Vec2<float>(textX, textCenterY), name, mainColor, textSize, alpha);

        if(mode != "NULL") {
            float modeX = textX + nameW;
            ImGuiUtils::drawText(Vec2<float>(modeX + 1.f, textCenterY + 1.f), " " + mode,
                                 UIColor(0, 0, 0, (int)(180 * alpha)), textSize);
            ImGuiUtils::drawText(Vec2<float>(modeX, textCenterY), " " + mode,
                                 UIColor(220, 220, 220, 255), textSize, alpha);
        }
    }
}
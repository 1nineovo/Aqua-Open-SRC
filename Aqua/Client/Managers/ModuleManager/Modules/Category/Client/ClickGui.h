#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../../../../Utils/ColorUtil.h"
#include "../../../../../../Utils/Maths.h"
#include "../../ModuleBase/Module.h"
#include "../../ModuleBase/Settings/TextSetting.h"

void getModuleListByCategory(Category category, std::vector<Module*>& modList);

struct ClickWindow {
    std::string windowName;
    Vec2<float> pos;
    float yOffset;
    bool extended = true;
    bool isDragging = false;
    Category category;
    std::vector<Module*> modList;

    ClickWindow(const Vec2<float>& Pos, const Category& c) {
        this->pos = Pos;
        this->extended = true;
        if(c == Category::COMBAT)
            this->windowName = "Combat";
        else if(c == Category::MISC)
            this->windowName = "Misc";
        else if(c == Category::RENDER)
            this->windowName = "Render";
        else if(c == Category::MOVEMENT)
            this->windowName = "Movement";
        else if(c == Category::PLAYER)
            this->windowName = "Player";
        else if(c == Category::CLIENT)
            this->windowName = "Client";

        getModuleListByCategory(c, modList);
    }
};
class ClickGui : public Module {
   public:
    ClickGui();
    ~ClickGui();

    std::vector<ClickWindow*> windowList;
    bool isLeftClickDown = false;
    bool isRightClickDown = false;
    bool isHoldingLeftClick = false;
    bool isHoldingRightClick = false;
    Vec2<float> startDragPos = Vec2<float>(0.f, 0.f);
    float openDuration = 0.f;
    bool initClickGui = false;
    void init();

    bool tooltips = false;
    bool isShiftDown = false;
    bool isBackKeyDown = false;
    std::string searchingModule;
    bool isSearching = false;
    int lastKeyPress = -1;
    bool* isChoosingKeyBindPtr = nullptr;

    float textSize = 1.f;

    Vec2<float> mousePos = Vec2<float>(0.f, 0.f);
    Vec2<float> mouseDelta = Vec2<float>(0.f, 0.f);

    std::unordered_map<Module*, float> animProgress;

    SliderSettingBase* draggingSliderSettingPtr = nullptr;
    KeybindSetting* capturingKbSettingPtr = nullptr;
    TextSetting* editingTextSettingPtr = nullptr;
    std::string textEditBuffer = "";

    void onMouseUpdate(Vec2<float> mousePos, char mouseButton, char isDown);
    void onKeyUpdate(int key, bool isDown) override;
    void render(ImDrawList* drawlist);
    virtual void onEnable() override;
    virtual void onDisable() override;
};

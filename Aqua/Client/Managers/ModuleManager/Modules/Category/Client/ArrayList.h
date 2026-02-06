#pragma once

#include "../../../../../../Utils/ColorUtil.h"
#include "../../ModuleBase/Module.h"

class Arraylist : public Module {
   private:
    int mode = 1;
    int offset = 10;
    int gap = 5;
    float size = 1.f;
    float bgOpacity = 0.6f;
    float rounding = 3.f;

   public:
    bool bottom = false;
    bool glow = false;
    Arraylist();
    void onImGuiRender(ImDrawList* drawlist);
};
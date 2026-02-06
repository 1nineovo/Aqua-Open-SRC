#pragma once
#include "../FuncHook.h"

class KeyMouseHook : public FuncHook {
private:
	using func_t = void(__fastcall*)(__int64, char, char, __int16, __int16, __int16, __int16, char);
	static inline func_t oFunc;

	static void mouseInputCallback(__int64 a1, char mouseButton, char isDown, __int16 mouseX, __int16 mouseY, __int16 relativeMovementX, __int16 relativeMovementY, char a8) {

		GC::keyMousePtr = (void*)(a1 + 0x10);

        static ClickGui* guiMod = ModuleManager::getModule<ClickGui>();
        static Editor* EditorMod = ModuleManager::getModule<Editor>();
        static Interface* interfaceMode = ModuleManager::getModule<Interface>();
     
         if(guiMod->isEnabled()) {
            guiMod->mousePos = Vec2<float>((float)mouseX, (float)mouseY);
            guiMod->onMouseUpdate(Vec2<float>((float)mouseX, (float)mouseY), mouseButton, isDown);
            return;
        }
        else if(EditorMod->isEnabled()) {
            EditorMod->onMouseUpdate(Vec2<float>((float)mouseX, (float)mouseY), mouseButton, isDown);
            return;
        }
        if(ImGui::GetCurrentContext() != nullptr) {
            ImGuiIO& io = ImGui::GetIO();

            if(mouseX != 0 && mouseY != 0) {
                io.MousePos = ImVec2(mouseX, mouseY);
            }

            switch(mouseButton) {
                case 1:
                    io.MouseDown[0] = isDown;
                    break;
                case 2:
                    io.MouseDown[1] = isDown;
                    break;
                case 3:
                    io.MouseDown[2] = isDown;
                    break;
                case 4:
                    if(isDown == 0x78 || isDown == 0x7F) {
                        io.AddMouseWheelEvent(0, 0.5);  // Scroll up
                    } else if(isDown == 0x88 || isDown == 0x80) {
                        io.AddMouseWheelEvent(0, -0.5);  // Scroll down
                    }
                    break;
                default:
                    break;
            }
        }
		oFunc(a1, mouseButton, isDown, mouseX, mouseY, relativeMovementX, relativeMovementY, a8);
	}
public:
	KeyMouseHook() {
		OriginFunc = (void*)&oFunc;
		func = (void*)&mouseInputCallback;
	}
};
#include "Swing.h"
#include <Minecraft/InvUtil.h>

Swing::Swing() : Module("Swing", "Swing animation", Category::RENDER) {
    registerSetting(new BoolSetting("Swing Speed", "", &swingSpeed, false));
	registerSetting(new SliderSetting<int>("Speed", "Swing speed", &speed, speed, 1, 20));
    registerSetting(new BoolSetting("Smooth", "", &fluxSwing, false));
    registerSetting(new BoolSetting("FakeBlock", "", &fakeBlock, false));
    registerSetting(new BoolSetting("SmallItem", "", &smallItem, false));
}
DEFINE_NOP_PATCH_FUNC(patchFluxSwing, MemoryUtil::findSignature("E8 ? ? ? ? 48 8B ? F3 0F ? ? ? ? ? ? F3 0F ? ? ? ? ? ? F3 0F ? ? ? ? ? ? C6 40 38 ? 48 8B ? EB"), 0x5);

void Swing::onDisable() {
    patchFluxSwing(false);
}

void Swing::onEnable() {
    patchFluxSwing(fluxSwing);
}


void Swing::onNormalTick(LocalPlayer* lp) {
    ItemStack* it = InvUtil::getItem(InvUtil::getSelectedSlot());
    std::string name = getItemNameLower(it ? it->getItem() : nullptr);
    if(fakeBlock && ImGui::IsMouseDown(ImGuiMouseButton_Right) &&(name.find("sword") != std::string::npos || name.find("axe") != std::string::npos ||
       name.find("pickaxe") != std::string::npos || name.find("shovel") != std::string::npos || name.find("hoe") != std::string::npos)) {
        shouldBlock = true;
    } else {
        shouldBlock = false;
    }
}

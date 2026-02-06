#include "CameraTweaks.h"

CameraTweaks::CameraTweaks() : Module("CameraTweaks", "Allows modification of the third person camera", Category::RENDER) {
	registerSetting(new BoolSetting("Clip", "Allows the camera to clip through blocks", &clip, false));
	registerSetting(new SliderSetting<float>("Distance", "The distance the third person camera is from the player", &distance, 4.f, 1.f, 50.f));

}

void CameraTweaks::onDisable() {

}

void CameraTweaks::onClientTick() {

}
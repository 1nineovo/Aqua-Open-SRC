#include "CustomFont.h"

CustomFont::CustomFont() : Module("Fonts", "Font of Client", Category::CLIENT) {
	registerSetting(new SliderSetting<int>("FontSize", "NULL", &fontSize, 25, 15, 40));
}

bool CustomFont::isEnabled() {
	return true;
}

bool CustomFont::isVisible() {
	return false;
}

std::string CustomFont::getSelectedFont() {
	return fontEnumSetting->enumList[fontMode];
}
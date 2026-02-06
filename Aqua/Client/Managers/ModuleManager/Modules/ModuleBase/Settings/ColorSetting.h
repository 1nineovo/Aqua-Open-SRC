#pragma once
#include <vector>
#include "Setting.h"
#include "SliderSetting.h"
#include "../../../../../../Utils/ColorUtil.h"

class ColorSetting : public Setting {
public:
	UIColor* colorPtr;
	std::vector<SliderSetting<uint8_t>*> colorSliders;

	bool extended = false;
    bool theme = false;

	ColorSetting(std::string settingName, std::string des, UIColor* ptr, UIColor defaultValue,
                 bool alpha = true,
                 std::optional<std::function<bool(void)>> _dependOn = std::nullopt) {
		this->name = settingName;
		this->description = des;
		this->colorPtr = ptr;
        this->dependOn = _dependOn;

		colorSliders.push_back(new SliderSetting<uint8_t>("Red", "NULL", &colorPtr->r, defaultValue.r, 0, 255));
		colorSliders.push_back(new SliderSetting<uint8_t>("Green", "NULL", &colorPtr->g, defaultValue.g, 0, 255));
		colorSliders.push_back(new SliderSetting<uint8_t>("Blue", "NULL", &colorPtr->b, defaultValue.b, 0, 255));

		if (alpha)
			colorSliders.push_back(new SliderSetting<uint8_t>("Alpha", "NULL", &colorPtr->a, defaultValue.a, 0, 255));

		this->type = SettingType::COLOR_S;
        static const Vec2<float> colorPickerSize = Vec2<float>(136.f, 100.f);
        static float h, s, v;
        ImGui::ColorConvertRGBtoHSV((float)colorPtr->r / 255.f, (float)colorPtr->g / 255.f,
                                    (float)colorPtr->b / 255.f, h, s, v);
        this->hueDuration = h;
        this->pos = Vec2<float>(s * colorPickerSize.x, (v - 1.f) * -1.f * colorPickerSize.y);
	}

	~ColorSetting() {
		for (auto& slider : colorSliders) {
			delete slider;
			slider = nullptr;
		}
		colorSliders.clear();
	}
};

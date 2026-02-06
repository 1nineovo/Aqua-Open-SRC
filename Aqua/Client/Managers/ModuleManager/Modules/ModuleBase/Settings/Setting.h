#pragma once
#include <string>
#include <Maths.h>

enum class SettingType {
	UNKNOW_S,
	BOOL_S,
	KEYBIND_S,
	ENUM_S,
	COLOR_S,
	SLIDER_S,
	PAGE_S,
	TEXT_S
};

class Setting {
public:
	std::string name;
	std::string description;
	SettingType type = SettingType::UNKNOW_S;
    std::optional<std::function<bool(void)>> dependOn = std::nullopt;
   public:
	float selectedAnim = 0.f;
    bool isDragging = false;
    bool isDragging2 = false;
    bool isDragging3 = false;
    Vec2<float> pos;
    float hueDuration = 0.f;
};

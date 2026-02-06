#pragma once
#include "Setting.h"
#include <string>

enum class SettingType;

class TextSetting : public Setting {
public:
    std::string* value;
    std::string defaultValue;
    int maxLength;

    TextSetting(std::string settingName, std::string des, std::string* ptr, 
                std::string defaultVal, int maxLen = 64,
                std::optional<std::function<bool(void)>> _dependOn = std::nullopt) {
        this->name = settingName;
        this->description = des;
        this->value = ptr;
        this->defaultValue = defaultVal;
        *this->value = defaultVal;
        this->maxLength = maxLen;
        this->type = SettingType::TEXT_S;
        this->dependOn = _dependOn;
    }
};

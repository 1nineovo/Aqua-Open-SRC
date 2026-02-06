#pragma once
#include "../../ModuleBase/Module.h"

class CustomFont : public Module {
public:
	EnumSetting* fontEnumSetting = nullptr;
	int fontMode = 1;
	int fontSize = 30;
	bool shadow = false;
	std::string getSelectedFont();
public:

	CustomFont();

	bool isEnabled() override;
	bool isVisible() override;
};

#include "Module.h"
#include<json.hpp>
#include "../Category/Client/Notifications.h"


using json = nlohmann::json;

Module::Module(std::string moduleName, std::string des, Category c, int k) {
	this->name = moduleName;
	this->description = des;
	this->category = c;
	this->keybind = k;

	registerSetting(new BoolSetting("Visible", "Visible on arraylist", &visible, true));
	registerSetting(new KeybindSetting("Keybind", "Keybind of module", &keybind, k));
	registerSetting(new EnumSetting("Toggle", "How module should be toggled", { "Press", "Hold" }, &toggleMode, 0));
}

Module::~Module() {
	for (auto& setting : settings) {
		delete setting;
		setting = nullptr;
	}
	settings.clear();
}

std::string Module::getModeText() {
	return "NULL";
}

bool Module::isEnabled() {
	return enabled;
}

bool Module::isVisible() {
	return visible;
}

bool Module::isHoldMode() {
	return toggleMode;
}

int Module::getKeybind() {
	return keybind;
}

void Module::setKeybind(int key) {
	this->keybind = key;
}

bool Module::runOnBackground() {
	return false;
}

void Module::setEnabled(bool enable) {
	if (this->enabled != enable) {
		this->enabled = enable;

        if(this->getModuleName() != "ClickGui") {
            std::string message = std::string(this->getModuleName()) +
                                  (enabled ? " has been enabled" : " has been disabled");
            Notifications::addNotifBox(message, 1.f);
        }

		if (enable) {
			this->onEnable();
		}
		else {
			this->onDisable();
		}
	}
}

void Module::toggle() {
	setEnabled(!enabled);
}

void Module::onDisable() {
}

void Module::onEnable() {
}

void Module::onKeyUpdate(int key, bool isDown) {
	if (getKeybind() == key) {
		if (isHoldMode()) {
			setEnabled(isDown);
		}
		else {
			if (isDown) {
				toggle();
			}
		}
	}
}

void Module::onClientTick() {
}

void Module::onNormalTick(LocalPlayer* localPlayer) {
}

void Module::onLevelTick(Level* level) {
}

void Module::onUpdateRotation(LocalPlayer* localPlayer) {
}

void Module::onSendPacket(Packet* packet) {
}

void Module::onReceivePacket(Packet* packet, bool *cancel) {
}

void Module::onD2DRender() {
}

void Module::onMCRender(MinecraftUIRenderContext* renderCtx) {
}

void Module::onImGuiRender(ImDrawList* drawlist) {}

void Module::onLevelRender() {
}
void Module::onRenderActorBefore(Actor* actor, Vec3<float>* camera, Vec3<float> *pos) {}
void Module::onRenderActorAfter(Actor* actor) {}

void Module::onChestScreen(ContainerScreenController* csc) {}

void Module::onLoadConfig(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	std::string modName = this->getModuleName();

	if (conf->contains(modName)) {
		json obj = conf->at(modName);
		if (obj.is_null())
			return;

		if (obj.contains("enabled")) {
			this->setEnabled(obj.at("enabled").get<bool>());
		}

		for (auto& setting : settings) {
			std::string settingName = setting->name;

			if (obj.contains(settingName)) {
				json confValue = obj.at(settingName);
				if (confValue.is_null())
					continue;

				switch (setting->type) {
				case SettingType::BOOL_S: {
					BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
					(*boolSetting->value) = confValue.get<bool>();
					break;
				}
				case SettingType::KEYBIND_S: {
					KeybindSetting* keybindSetting = static_cast<KeybindSetting*>(setting);
					(*keybindSetting->value) = confValue.get<int>();
					break;
				}
				case SettingType::ENUM_S: {
					EnumSetting* enumSetting = static_cast<EnumSetting*>(setting);
					(*enumSetting->value) = confValue.get<int>();
					break;
				}
                case SettingType::COLOR_S: {
                    ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);
                    if(confValue.is_string()) {
                        (*colorSetting->colorPtr) =
                            ColorUtil::HexStringToColor(confValue.get<std::string>());
                        colorSetting->theme = false;
                    } else if(confValue.is_object()) {
                        if(confValue.contains("hex")) {
                            (*colorSetting->colorPtr) =
                                ColorUtil::HexStringToColor(confValue.at("hex").get<std::string>());
                        }
                        if(confValue.contains("theme")) {
                            colorSetting->theme = confValue.at("theme").get<bool>();
                        }
                    }
                    break;
                }
				case SettingType::SLIDER_S: {
					SliderSettingBase* sliderSettingBase = static_cast<SliderSettingBase*>(setting);
					if (sliderSettingBase->valueType == ValueType::INT_T) {
						SliderSetting<int>* intSlider = static_cast<SliderSetting<int>*>(sliderSettingBase);
						(*intSlider->valuePtr) = confValue.get<int>();
					}
					else if (sliderSettingBase->valueType == ValueType::FLOAT_T) {
						SliderSetting<float>* floatSlider = static_cast<SliderSetting<float>*>(sliderSettingBase);
						(*floatSlider->valuePtr) = confValue.get<float>();
					}
					break;
				}
				}
			}
		}
	}
}

void Module::onSaveConfig(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	std::string modName = this->getModuleName();
	json obj = (*conf)[modName];

	obj["enabled"] = this->isEnabled();

	for (auto& setting : settings) {
		std::string settingName = setting->name;

		switch (setting->type) {
		case SettingType::BOOL_S: {
			BoolSetting* boolSetting = static_cast<BoolSetting*>(setting);
			obj[settingName] = (*boolSetting->value);
			break;
		}
		case SettingType::KEYBIND_S: {
			KeybindSetting* keybindSetting = static_cast<KeybindSetting*>(setting);
			obj[settingName] = (*keybindSetting->value);
			break;
		}
		case SettingType::ENUM_S: {
			EnumSetting* enumSetting = static_cast<EnumSetting*>(setting);
			obj[settingName] = (*enumSetting->value);
			break;
		}
        case SettingType::COLOR_S: {
            ColorSetting* colorSetting = static_cast<ColorSetting*>(setting);

            json colorObj;
            colorObj["hex"] = ColorUtil::ColorToHexString((*colorSetting->colorPtr));
            colorObj["theme"] = colorSetting->theme; 

            obj[settingName] = colorObj;
            break;
        }
		case SettingType::SLIDER_S: {
			SliderSettingBase* sliderSettingBase = static_cast<SliderSettingBase*>(setting);
			if (sliderSettingBase->valueType == ValueType::INT_T) {
				SliderSetting<int>* intSlider = static_cast<SliderSetting<int>*>(sliderSettingBase);
				obj[settingName] = (*intSlider->valuePtr);
			}
			else if (sliderSettingBase->valueType == ValueType::FLOAT_T) {
				SliderSetting<float>* floatSlider = static_cast<SliderSetting<float>*>(sliderSettingBase);
				obj[settingName] = (*floatSlider->valuePtr);
			}
			break;
		}
		}
	}

	(*conf)[modName] = obj;
}
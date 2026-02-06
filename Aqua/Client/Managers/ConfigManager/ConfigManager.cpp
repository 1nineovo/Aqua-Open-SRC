#include "ConfigManager.h"
#include "../../Client.h"
#include "../../../Utils/FileUtil.h"

#include "../ModuleManager/ModuleManager.h"

void ConfigManager::init() {
	configsPath = FileUtil::getClientPath() + "Configs\\";
	if (!FileUtil::doesFilePathExist(configsPath))
		FileUtil::createPath(configsPath);
}

void ConfigManager::deleteConfig(const std::string& name) {
    if(!doesConfigExist(name))
        return;
    std::string path = configsPath + name + configFormat;
    std::remove(path.c_str());
    Client::DisplayClientMessage("Successfully deleted config %s!", MCTF::GRAY, name.c_str());
}

std::vector<std::string> ConfigManager::getConfigList() {
    std::vector<std::string> list;
    try {
        if(!std::filesystem::exists(configsPath))
            return list;
        for(const auto& entry : std::filesystem::directory_iterator(configsPath)) {
            if(entry.path().extension() == configFormat) {
                list.push_back(entry.path().stem().string());
            }
        }
    } catch(...) {
    }
    return list;
}

bool ConfigManager::doesConfigExist(const std::string& name) {
	std::string path = configsPath + name + configFormat;
	return FileUtil::doesFilePathExist(path);
}

void ConfigManager::createNewConfig(const std::string& name) {
	if (doesConfigExist(name)) {
		Client::DisplayClientMessage("%sFailed to create config %s%s%s. Config already existed!", MCTF::RED, MCTF::GRAY, name.c_str(), MCTF::RED);
		return;
	}

	if (currentConfig != "NULL") {
		saveConfig();
	}

	currentConfig = name;
	saveConfig();

	Client::DisplayClientMessage("Successfully created config %s%s%s!", MCTF::GRAY, name.c_str(), MCTF::WHITE);
}

void ConfigManager::loadConfig(const std::string& name) {
	if (!doesConfigExist(name)) {
		Client::DisplayClientMessage("%sFailed to load config %s%s%s. Config doesn't exist!", MCTF::RED, MCTF::GRAY, name.c_str(), MCTF::RED);
		return;
	}

	currentConfig = name;

	std::string configPath = configsPath + name + configFormat;

	std::ifstream confFile(configPath.c_str(), std::ifstream::binary);
	currentConfigObj.clear();
	confFile >> currentConfigObj;
	ModuleManager::onLoadConfig((void*)&currentConfigObj);

	Client::DisplayClientMessage("Successfully loaded config %s%s%s!", MCTF::GRAY, name.c_str(), MCTF::WHITE);
}

void ConfigManager::saveConfig() {
	if (currentConfig == "NULL")
		return;

	std::string configPath = configsPath + currentConfig + configFormat;

	ModuleManager::onSaveConfig((void*)&currentConfigObj);

	std::ofstream o(configPath, std::ifstream::binary);
	o << std::setw(4) << currentConfigObj << std::endl;
	o.flush();
	o.close();
}
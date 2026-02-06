#include "ToggleCommand.h"

#include <algorithm>

#include "../../../Client.h"
#include "../../ModuleManager/ModuleManager.h"
#include "../../ModuleManager/Modules/ModuleBase/Module.h"

ToggleCommand::ToggleCommand()
    : CommandBase("toggle", "Toggles a module on or off", "<module>", {"t"}) {}

bool ToggleCommand::execute(const std::vector<std::string>& args) {
    if(args.size() < 2)
        return false;

    std::string moduleNeedToFind = args[1];
    std::transform(moduleNeedToFind.begin(), moduleNeedToFind.end(), moduleNeedToFind.begin(),
                   ::tolower);
    Module* currentModule = nullptr;

    for(auto& mod : ModuleManager::moduleList) {
        std::string moduleName = mod->getModuleName();
        std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);
        if(moduleName == moduleNeedToFind) {
            currentModule = mod;
            break;
        }
    }

    if(currentModule == nullptr) {
        Client::DisplayClientMessage("Couldn't find module with name: %s%s", MCTF::GRAY,
                                     moduleNeedToFind.c_str());
        return true;
    }

    currentModule->toggle();

    bool isEnabled = currentModule->isEnabled();

    const char* stateColor = isEnabled ? MCTF::GREEN : MCTF::RED;
    const char* stateText = isEnabled ? "Enabled" : "Disabled";

    Client::DisplayClientMessage("%s%s%s was %s%s%s", MCTF::GRAY,
                                 currentModule->getModuleName().c_str(), MCTF::RESET, stateColor,
                                 stateText, MCTF::RESET);

    return true;
}
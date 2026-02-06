#pragma once
#include "CommandBase.h"

class ToggleCommand : public CommandBase {
   public:
    ToggleCommand();
    bool execute(const std::vector<std::string>& args) override;
};
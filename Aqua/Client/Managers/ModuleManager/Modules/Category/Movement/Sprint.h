#pragma once
#include "../../ModuleBase/Module.h"

class Sprint : public Module {
   private:
   public:
    Sprint();
    virtual void onNormalTick(LocalPlayer* lp) override;
};

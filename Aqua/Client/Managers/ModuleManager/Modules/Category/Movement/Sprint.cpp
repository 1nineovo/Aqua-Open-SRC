#include "Sprint.h"

Sprint::Sprint() : Module("Sprint", "", Category::MOVEMENT, 0) {}

void Sprint::onNormalTick(LocalPlayer* lp) {
    if(lp == nullptr)
        return;
    if(lp->getMoveInputComponent() == nullptr)
        return;
    lp->getMoveInputComponent()->mIsSprinting = true;
}

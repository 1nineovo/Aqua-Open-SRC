#pragma once

#include "../../ModuleBase/Module.h"
#include <vector>
#include <algorithm>
#include "../../../../../../Utils/TimerUtil.h"
#include "../../../../../../Utils/Minecraft/WorldUtil.h"
#include "../../../../../../SDK/NetWork/Packets/PacketID.h"

class ChestAura : public Module {
public:
    ChestAura();

    void onEnable() override;
    void onDisable() override;
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onReceivePacket(Packet* packet, bool* cancel) override;

private:
    float mRangeValue = 4.f; // configurable range in blocks
    int mDelayValue = 500;   // delay between chest opens in ms

    std::vector<BlockPos> mOpenedChestPositions;
    bool mIsChestOpened = false;
    unsigned long long mTimeOfLastChestOpen = 0;
}; 
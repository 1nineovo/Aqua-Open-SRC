#include "ChestAura.h"

#include "../../../../../../Utils/Maths.h"
#include "../../../../../../Utils/TimerUtil.h"
#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../Utils/Minecraft/WorldUtil.h"
#include "../../../../../../SDK/NetWork/Packets/Packet.h"
#include <limits>

ChestAura::ChestAura()
    : Module("ChestAura", "Automatically opens nearby chests", Category::PLAYER) {
    registerSetting(new SliderSetting<float>("Range", "Search radius", &mRangeValue, mRangeValue, 1.f, 12.f));
    registerSetting(new SliderSetting<int>("Delay", "Delay between opens (ms)", &mDelayValue, mDelayValue, 0, 2000));
}

void ChestAura::onEnable() {
    mOpenedChestPositions.clear();
    mIsChestOpened = false;
    mTimeOfLastChestOpen = 0;
}

void ChestAura::onDisable() {
    mOpenedChestPositions.clear();
    mIsChestOpened = false;
}

void ChestAura::onNormalTick(LocalPlayer* localPlayer) {
    if(!localPlayer)
        return;

    unsigned long long now = TimerUtil::getCurrentMs();
    if(now - mTimeOfLastChestOpen < static_cast<unsigned long long>(mDelayValue))
        return;

    if(GI::getClientInstance()->getScreenName() != "hud_screen")
        return;

    const Vec3<float> playerPos = localPlayer->getPos();
    BlockSource* region = GI::getRegion();
    if(!region)
        return;

    std::vector<BlockPos> candidateChests;
    int searchRange = static_cast<int>(mRangeValue);

    for(int x = static_cast<int>(playerPos.x) - searchRange; x <= static_cast<int>(playerPos.x) + searchRange; ++x) {
        for(int y = static_cast<int>(playerPos.y) - searchRange; y <= static_cast<int>(playerPos.y) + searchRange; ++y) {
            for(int z = static_cast<int>(playerPos.z) - searchRange; z <= static_cast<int>(playerPos.z) + searchRange; ++z) {
                BlockPos pos(x, y, z);
                Block* block = region->getBlock(pos);
                if(!block || !block->blockLegcy)
                    continue;

                const std::string& name = block->blockLegcy->mName;
                if(name.find("chest") == std::string::npos)
                    continue;

                if(std::find(mOpenedChestPositions.begin(), mOpenedChestPositions.end(), pos) != mOpenedChestPositions.end())
                    continue;

                candidateChests.push_back(pos);
            }
        }
    }

    if(candidateChests.empty() || mIsChestOpened)
        return;

    std::sort(candidateChests.begin(), candidateChests.end(), [&](const BlockPos& a, const BlockPos& b) {
        return playerPos.dist(a.CastTo<float>()) < playerPos.dist(b.CastTo<float>());
    });

    BlockPos targetChest = candidateChests.front();

    int nearestFace = -1;
    float minDist = std::numeric_limits<float>::max();
    for(const auto& [face, offset] : WorldUtil::blockFaceOffsets) {
        Vec3<float> facePos = targetChest.CastTo<float>() - offset.CastTo<float>();
        float d = playerPos.dist(facePos);
        if(d < minDist) {
            minDist = d;
            nearestFace = face;
        }
    }

    if(nearestFace == -1)
        return;

    localPlayer->gamemode->buildBlock(targetChest, static_cast<unsigned char>(nearestFace), false);
    mOpenedChestPositions.push_back(targetChest);
    mTimeOfLastChestOpen = now;
}

void ChestAura::onReceivePacket(Packet* packet, bool* /*cancel*/) {
    if(!packet)
        return;

    PacketID id = packet->getId();
    if(id == PacketID::ChangeDimension) {
        mOpenedChestPositions.clear();
        mIsChestOpened = false;
    } else if(id == PacketID::ContainerOpen) {
        mIsChestOpened = true;
    } else if(id == PacketID::ContainerClose) {
        mIsChestOpened = false;
    }
} 
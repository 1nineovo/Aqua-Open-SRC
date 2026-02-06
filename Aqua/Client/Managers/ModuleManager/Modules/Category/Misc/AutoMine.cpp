
#include "AutoMine.h"
#include <Minecraft/WorldUtil.h>
#include <Minecraft/TargetUtil.h>
#include "../../../ModuleManager.h"


AutoMine::AutoMine() : Module("AutoMine", "Automatically mines their surround", Category::COMBAT) {
    registerSetting(new SliderSetting<float>("Range", "NULL", &range, 6.f, 1.f, 12.f));
    registerSetting(new BoolSetting("Burrow", "Mine their burrow", &burrow, false));
    registerSetting(new BoolSetting("Priority", "priority mining surr before packetmine", &priority, false));

}

BlockPos AutoMine::getMinePos(Actor* actor) const {
    static std::vector<BlockPos> mineList;
    mineList.clear();
    static BlockPos checkList[4] = {{1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}};
    const Vec3<float> actorPos = actor->getPos().floor();
    if(burrow && !WorldUtil::canBuildOn(actorPos.CastTo<int>()))
        return actorPos.CastTo<int>();
    for(const BlockPos& check : checkList) {
        const BlockPos& pos = actorPos.CastTo<int>().add(check);
        Block* block = WorldUtil::getBlock(pos);
        if(block->getblockLegcy()->blockid == 49) {
            mineList.push_back(pos);
        }
    }
    std::sort(mineList.begin(), mineList.end(), [&](BlockPos b1, BlockPos b2) {
        return WorldUtil::distanceToBlock(actorPos, b1) < WorldUtil::distanceToBlock(actorPos, b2);
    });
    if(mineList.empty())
        return BlockPos(0, 0, 0);
    return mineList[0];
}

static std::vector<Actor*> targetListAC;

void AutoMine::onNormalTick(LocalPlayer* localPlayer) {

    if(localPlayer == nullptr)
        return;
    targetListAC.clear();

    Level* level = localPlayer->level;
    if(level == nullptr)
        return;

    for(Actor* actor : level->getRuntimeActorList()) {
        if(!TargetUtil::isTargetValid(actor, false))
            continue;
        if(localPlayer->getPos().dist(actor->getPos()) <= range)
            targetListAC.push_back(actor);
    }

    if(targetListAC.empty())
        return;
    std::sort(targetListAC.begin(), targetListAC.end(), [](Actor* a1, Actor* a2) {
        Vec3<float> lpPos = GI::getLocalPlayer()->getPos();
        return ((a1->getPos().dist(lpPos)) < (a2->getPos().dist(lpPos)));
    });

    GameMode* gm = localPlayer->gamemode;
    if(gm == nullptr)
        return;

    auto packetMine = ModuleManager::getModule<PacketMine>();
    if(packetMine == nullptr)
        return;

    if(packetMine->getBreakPos() != BlockPos(0, 0, 0))
        return;

    BlockPos enemyPos = targetListAC[0]->getPos().floor().CastTo<int>();
    enemyPos.y -= 1;  // Foot level

    BlockPos surround[4] = {enemyPos.add(BlockPos(1, 0, 0)), enemyPos.add(BlockPos(-1, 0, 0)),
                            enemyPos.add(BlockPos(0, 0, 1)), enemyPos.add(BlockPos(0, 0, -1))};

    BlockSource* region = GI::getRegion();
    if(!region)
        return;

    for(const BlockPos& pos : surround) {
        Block* block = region->getBlock(pos);
        if(!block || !block->getblockLegcy())
            continue;
        int id = block->getblockLegcy()->blockid;
        if(id == 0 || id == 7)
            continue;  // Skip air and bedrock

        bool dummy = true;
        gm->startDestroyBlock(pos, 0, dummy);
        packetMine->mineBlock(pos, 0);
    }
}

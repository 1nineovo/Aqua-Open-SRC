#pragma once
#pragma once
#include <algorithm>
#include <vector>
#include "../../../../../../Utils/Maths.h"  // for Vec2
#include "../../../../../../Utils/Minecraft/PlayerUtil.h"
#include "../../../../../../Utils/Minecraft/WorldUtil.h"
#include "../../../ModuleManager.h"
#include "../../ModuleBase/Module.h"

class PistonCrystal : public Module {
   private:
    int range = 5;
    int delay = 3;
    int delay_rot = 1;
    bool silent = false;
    bool self = true;
    bool toggle = false;
    bool use_inv = false;
    bool eatstop = true;

    bool render = true;
    UIColor pistonColor = UIColor(0, 255, 0, 60);
    UIColor redstoneColor = UIColor(255, 0, 0, 60);
    std::vector<std::pair<BlockPos, float>> pistonRenderList;
    std::vector<std::pair<BlockPos, float>> redstoneRenderList;

    int delay_count = 0;
    int delay_rot_count = 0;
    bool placed = false;

    std::vector<Actor*> target;
    std::vector<Actor*> target_crystal;

    enum Rot { Xplus = 0, Xminus = 1, Zplus = 2, Zminus = 3, RESET = 4 };

    Vec2<float> angle = {0, 0};
    Vec2<float> angle_reset = {0, 0};

    const Vec3<float> poslist[5] = {
        {1, 0, 0},   // X+
        {-1, 0, 0},  // X-
        {0, 0, 1},   // Z+
        {0, 0, -1},  // Z-
        {0, 0, 0}    // Reset
    };

    using BlockPos = Vec3<int>;

    bool isInRenderList(const std::vector<std::pair<BlockPos, float>>& list, const BlockPos& pos);

   public:
    static inline PistonCrystal* instance;
    PistonCrystal();

    virtual void onEnable() override;
    virtual void onNormalTick(LocalPlayer* localPlayer) override;
    virtual void onSendPacket(Packet* packet) override;

    void _main(const BlockPos& pos);
    bool can(const BlockPos& pos, bool crystal);
    bool check_crystal();
    void setAngle(const BlockPos& targetPos);
    void destroy();
    void reset();
    void place(BlockPos crystal, BlockPos piston, BlockPos redstone);
    void setAngle(Rot _mode);


    bool x_plus(BlockPos pos, bool checking = false);
    bool x_minus(BlockPos pos, bool checking = false);
    bool z_plus(BlockPos pos, bool checking = false);
    bool z_minus(BlockPos pos, bool checking = false);
    
};
#include "Clip.h"
#include <DrawUtil.h>

Clip::Clip() : Module("Clip", "Smart clipping with directional rendering", Category::MOVEMENT) {
    registerSetting(new SliderSetting<float>("Reduce", "Reduce hitbox size (0.0-0.29)", &offset, offset, 0.0f, 0.29f));
    registerSetting(new BoolSetting("Show Hitbox", "Show modified hitbox", &showHitbox, showHitbox));
}

void Clip::onEnable() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if (!localPlayer) return;
    
    originalAABB = localPlayer->getAABB(true);
    hasOriginalAABB = true;
}

void Clip::onDisable() {
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if (!localPlayer || !hasOriginalAABB) return;
    
    localPlayer->setAABB(originalAABB);
    hasOriginalAABB = false;
}

bool Clip::isBlockSolid(BlockPos pos) {
    BlockSource* region = GI::getRegion();
    if (!region) return false;
    
    Block* block = region->getBlock(pos);
    if (!block || !block->blockLegcy) return false;
    
    uint16_t blockId = block->blockLegcy->blockid;
    return blockId != 0 && !(blockId >= 8 && blockId <= 11); // 排除水和岩浆
}

Vec3<float> Clip::getPlayerDirection(LocalPlayer* player) {
    Vec2<float> rotation = player->getRotation();
    float yaw = rotation.y * (3.14159f / 180.0f); // 转换为弧度
    
    return Vec3<float>(-sinf(yaw), 0.0f, cosf(yaw));
}

void Clip::onNormalTick(LocalPlayer* localPlayer) {
    if (!localPlayer) return;
    
    AABB currentAABB = localPlayer->getAABB(true);
    Vec3<float> playerPos = localPlayer->getPos();
    
    float standardWidth = 0.6f;
    float newWidth = standardWidth * (1.0f - offset);
    if (newWidth < 0.01f) newWidth = 0.01f;
    
    float halfWidth = newWidth * 0.5f;
    
    AABB newAABB;
    newAABB.lower.x = playerPos.x - halfWidth;
    newAABB.lower.y = currentAABB.lower.y;
    newAABB.lower.z = playerPos.z - halfWidth;
    
    newAABB.upper.x = playerPos.x + halfWidth;
    newAABB.upper.y = currentAABB.upper.y;
    newAABB.upper.z = playerPos.z + halfWidth;
    
    localPlayer->setAABB(newAABB);
}

void Clip::onLevelRender() {
    if (!showHitbox) return;
    
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if (!localPlayer) return;
    
    Vec3<float> playerPos = localPlayer->getPos();
    AABB playerAABB = localPlayer->getAABB(true);
    
    BlockPos playerBlockPos = Vec3<float>(playerPos.x, playerAABB.lower.y, playerPos.z).floor().CastTo<int>();
    
    struct Direction {
        Vec3<int> offset;
        std::string name;
        UIColor color;
    };
    
    Direction directions[] = {
        {{1, 0, 0}, "East", UIColor(255, 0, 0, 120)},    // 红色 - 东
        {{-1, 0, 0}, "West", UIColor(0, 255, 0, 120)},   // 绿色 - 西
        {{0, 0, 1}, "South", UIColor(0, 0, 255, 120)},   // 蓝色 - 南
        {{0, 0, -1}, "North", UIColor(255, 255, 0, 120)} // 黄色 - 北
    };
    
    for (const auto& dir : directions) {
        for (int y = 0; y < 2; y++) {
            BlockPos checkPos = playerBlockPos.add(dir.offset).add(Vec3<int>(0, y, 0));
            
            if (isBlockSolid(checkPos)) {
                AABB blockAABB;
                blockAABB.lower = checkPos.CastTo<float>();
                blockAABB.upper = blockAABB.lower.add(Vec3<float>(1.0f, 1.0f, 1.0f));
                
                bool intersects = false;
                
                AABB expandedPlayerAABB = playerAABB;
                float checkDistance = 0.1f; // 检查距离
                expandedPlayerAABB.lower = expandedPlayerAABB.lower.sub(Vec3<float>(checkDistance, 0, checkDistance));
                expandedPlayerAABB.upper = expandedPlayerAABB.upper.add(Vec3<float>(checkDistance, 0, checkDistance));
                
                bool xOverlap = expandedPlayerAABB.lower.x < blockAABB.upper.x && expandedPlayerAABB.upper.x > blockAABB.lower.x;
                bool yOverlap = expandedPlayerAABB.lower.y < blockAABB.upper.y && expandedPlayerAABB.upper.y > blockAABB.lower.y;
                bool zOverlap = expandedPlayerAABB.lower.z < blockAABB.upper.z && expandedPlayerAABB.upper.z > blockAABB.lower.z;
                
                intersects = xOverlap && yOverlap && zOverlap;
                
                if (intersects) {
                    UIColor lineColor = dir.color;
                    lineColor.a = 255; // 边框不透明
                    
                    DrawUtil::drawBox3dFilled(blockAABB, dir.color, lineColor, 1.0f);
                    break; // 找到一个就够了，避免重复渲染
                }
            }
        }
    }
    
}
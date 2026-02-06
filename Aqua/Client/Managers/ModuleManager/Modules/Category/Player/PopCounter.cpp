#include "PopCounter.h"

#include <AnimationUtil.h>
#include <ColorUtil.h>
#include <DrawUtil.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../../../../SDK/GlobalInstance.h"
#include "../../../../../../SDK/NetWork/MinecraftPacket.h"
#include "../../../../../../SDK/NetWork/Packets/PacketID.h"
#include "../../../../../../SDK/NetWork/Packets/TextPacket.h"
#include "../../../../../../SDK/World/Item/Item.h"
#include "../../../../../../Utils/Minecraft/TargetUtil.h"
#include "../../../../../../Utils/Minecraft/WorldUtil.h"
#include "../../../../../Client.h"

using namespace std;

PopCounter::PopCounter() : Module("PopCounter", "PopCounter", Category::PLAYER) {
    registerSetting(new BoolSetting("SendChat", "NULL", &sendchat, false));
    registerSetting(new BoolSetting("PopCham", "NULL", &popcham, true));

    srand(static_cast<unsigned int>(time(nullptr)));
}

static std::vector<Actor*> playerlist;

void sendMessage1(const std::string& str) {
    auto packet = MinecraftPacket::createPacket<TextPacket>();
    auto* pkt = packet.get();

    pkt->mType = TextPacketType::Chat;
    pkt->mMessage = str;
    pkt->mPlatformId = "";
    pkt->mLocalize = false;
    pkt->mXuid = "";
    pkt->mAuthor = "";

    GI::getPacketSender()->sendToServer(pkt);
}

static std::unordered_map<std::string, int> popCounts;
static std::unordered_map<std::string, bool> lastHadTotem;

void PopCounter::onNormalTick(LocalPlayer* localPlayer) {
    playerlist.clear();
    Level* level = localPlayer->level;
    if(!level)
        return;
    if(localPlayer == nullptr)
        return;

    if(!localPlayer->isAlive()) {
        popCounts.clear();
        lastHadTotem.clear();
    }

    for(auto& entity : level->getRuntimeActorList()) {
        if(TargetUtil::isTargetValid(entity, false,false)) {
            float rangeCheck = 100;
            if(WorldUtil::distanceToEntity(localPlayer->getPos(), entity) <= rangeCheck) {
                playerlist.push_back(entity);
            }
        }
    }

    for(auto* player : playerlist) {
        std::string playerName = player->getNameTag().c_str();

        bool hasTotem = false;
        auto* offhand = player->getOffhandSlot();
        if(offhand) {
            Item* item = offhand->getItem();
            if(item && item->mItemId == 601) {
                hasTotem = true;
            }
        }

        if(lastHadTotem[playerName] && !hasTotem) {
            popCounts[playerName] += 1;
            Client::DisplayClientMessage("%s Popped %d Totems!", playerName.c_str(),
                                         popCounts[playerName]);
            if(sendchat) {
                std::string message = "> @" + playerName + " popped " +
                                      std::to_string(popCounts[playerName]) +
                                      " totems thanks to Aqua!";
                sendMessage1(message);
            }

            if(popcham) {
                Vec3<float> popPos = player->getPos().add(Vec3<float>(0, 0, 0));
                float pitch = player->getActorRotationComponent()->mPitch;
                float yaw = player->getActorRotationComponent()->mYaw;

                popAnimations.emplace_back(popPos, pitch, yaw);
            }
        }

        lastHadTotem[playerName] = hasTotem;
    }
}

void PopCounter::onLevelRender() {
    if(!popcham)
        return;
    if(!GI::isInHudScreen())
        return;
    LocalPlayer* localPlayer = GI::getLocalPlayer();
    if(!localPlayer)
        return;

    float currentTime = GetTickCount64() / 1000.0f;

    for(auto it = popAnimations.begin(); it != popAnimations.end();) {
        PopAnimationData& anim = *it;

        float elapsed = currentTime - anim.startTime;
        float progress = elapsed / anim.duration;

        if(progress >= 1.0f) {
            it = popAnimations.erase(it);
            continue;
        }
        float spantime = GetTickCount64() / 500.0f;
     
        float riseHeight = 3.0f;
        float currentHeight = AnimationUtil::lerp(0.0f, riseHeight, progress);
        anim.currentPos = anim.startPos.add(Vec3<float>(0, currentHeight, 0));

        Vec3<float> finalPos = anim.currentPos;

        float alphaProgress = 1.0f - progress;

        UIColor currentColor = anim.color;
        currentColor.a = (uint8_t)(anim.color.a * alphaProgress);

        DrawUtil::drawPlayerModel(finalPos, anim.pitch, anim.yaw, currentColor,
                                  UIColor(0, 0, 0, 0));
     
        ++it;
    }
}
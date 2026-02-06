#include "AutoCrystal.h"
#include <cmath>
#include <Minecraft/InvUtil.h>
#include <Minecraft/TargetUtil.h>
UIColor lineColor;

AutoCrystal::AutoCrystal()
    : Module("AutoCrystal", "Automatically breaks and places Crystal", Category::COMBAT) {
    registerSetting(new SliderSetting<int>("TargetRange", "Range for targeting entities",
                                           &targetRange, 10, 1, 20));
    registerSetting(new BoolSetting("AutoPlace", "Place End Crystals at Target", &place, true));
    registerSetting(new BoolSetting("AutoBreak", "Explode End Crystals at Target", &abreak, true));

    registerSetting(
        new SliderSetting<int>("PlaceRange", "Range for placing crystals", &placeRange, 5, 1, 12));
    registerSetting(new SliderSetting<int>("PlaceDelay", "Delay between placing crystals (ticks)",
                                           &placeDelay, 5, 0, 20));

    registerSetting(new SliderSetting<float>("Proximity", "Proximity for crystal placement",
                                             &proximity, 6.f, 1.f, 12.f));
    registerSetting(new SliderSetting<float>("Max Self Damage", "Maximum damage to self",
                                             &localDamage, 4.f, 0.f, 36.f));
    registerSetting(new SliderSetting<float>("Min Enemy Damage", "Minimum damage to enemy",
                                             &enemyDamage, 8.f, 0.f, 36.f));
    registerSetting(
        new SliderSetting<int>("Quantity", "Number of crystals to place", &wasteAmount, 3, 1, 10));

    registerSetting(
        new SliderSetting<int>("BreakRange", "Range for breaking crystals", &breakRange, 5, 1, 12));
    registerSetting(new SliderSetting<int>("BreakDelay", "Delay between breaking crystals (ticks)",
                                           &breakDelay, 5, 0, 20));

    registerSetting(
        new BoolSetting("IdPredict", "Predicts where to attack ahead of time.", &predict, false));
    registerSetting(new SliderSetting<int>("Packets", "Number of packets for prediction",
                                           &predictPacket, 5, 1, 30));
    registerSetting(new SliderSetting<int>(
        "Send Delay", "Delay (in ticks) between sending packets.", &sendDelay, 5, 1, 10));

    registerSetting(new SliderSetting<float>("Max Self Break Damage",
                                             "Maximum damage to self when breaking",
                                             &breakLocalDamage, 4.f, 0.f, 36.f));
    registerSetting(new SliderSetting<float>("Min Enemy Break Damage",
                                             "Minimum damage to enemy when breaking",
                                             &breakEnemyDamage, 8.f, 0.f, 36.f));

    registerSetting(new EnumSetting("SwitchMode", "Crystal switching mode",
                                    {"None", "Swap", "SwapBack"}, &switchMode, 2));
    registerSetting(
        new BoolSetting("EatStop", "Multitasks like eating and crystalling", &eatstop, true));

    registerSetting(new EnumSetting("Render", "Rendering mode for placements",
                                    {"Off", "Box", "Flat"}, &renderType, 0));
    registerSetting(new ColorSetting("Color", "Render color", &renderColor, {255, 0, 0}));
    registerSetting(
        new ColorSetting("Line Color", "render  line color", &lineColor, {255, 255, 255, 85}, 6));
    registerSetting(
        new BoolSetting("RenderDamage", "Display damage dealt during render", &dmgText, true));

    registerSetting(new SliderSetting<float>(
        "Fade Duration", "Duration of placement fade animation", &fadeDur, 1.f, 0.2f, 3.f));

    registerSetting(new BoolSetting("SelfTest", "Enable testing on yourself", &selfTest, false));
}

bool AutoCrystal::sortCrystal(CrystalData c1, CrystalData c2) {
    float diff1 = c1.targetDamage - c1.localDamage;
    float diff2 = c2.targetDamage - c2.localDamage;
    return diff1 > diff2;
}

float calculateDamage(const BlockPos& crystalPos, Actor* target) {
    Vec3<float> crystalPosFloat(static_cast<float>(crystalPos.x) + 0.5f,
                                static_cast<float>(crystalPos.y) + 1.0f,
                                static_cast<float>(crystalPos.z) + 0.5f);
    return DamageUtils::getExplosionDamage(crystalPosFloat, target);
}

bool AutoCrystal::isPlaceValid(const BlockPos& blockPos, Actor* actor) {
    BlockLegacy* block = GI::getRegion()->getBlock(blockPos)->getblockLegcy();
    if(!(block->blockid == 7 || block->blockid == 49))
        return false;
    Vec3<float> blockPosFloat(blockPos.x, blockPos.y, blockPos.z);
    Vec3<float> pos = Vec3<float>(blockPos.x + 0.5f, blockPos.y + 1.0f, blockPos.z + 0.5f);
    if(GI::getLocalPlayer()->getEyePos().dist(pos) > placeRange)
        return false;
    if(!(GI::getClientInstance()->getRegion()->getBlock(blockPos)->getblockLegcy()->blockid == 49 ||
         GI::getClientInstance()->getRegion()->getBlock(blockPos)->getblockLegcy()->blockid == 7))
        return false;
    if(!GI::getClientInstance()
            ->getRegion()
            ->getBlock(blockPos.add2(0, 1, 0))
            ->getblockLegcy()
            ->blockid == 0)
        return false;
    if(!GI::getClientInstance()
            ->getRegion()
            ->getBlock(blockPos.add2(0, 2, 0))
            ->getblockLegcy()
            ->blockid == 0)
        return false;
    Vec3<float> lower(blockPos.x, blockPos.y, blockPos.z);
    Vec3<float> upper(blockPos.x + 1, blockPos.y + 2, blockPos.z + 1);
    const AABB blockAABB(lower, upper);

    for(Actor* entity : entityList) {
        if(entity->getActorTypeComponent()->mType != ActorType::EnderCrystal)
            continue;  // Skip crystals
        AABB entityAABB = entity->getAABBShapeComponent()->getAABB();
        if(entity->getActorTypeComponent()->mType != ActorType::Player ||
           entity == GI::getLocalPlayer()) {
            Vec3<float> pos = entity->getStateVectorComponent()->mPos;
            entityAABB.lower = pos.sub(Vec3<float>(0.3f, 0.0f, 0.3f));
            entityAABB.upper = pos.add(Vec3<float>(0.3f, 1.8f, 0.3f));
            entityAABB = entityAABB.expand(0.1f);
        }

        if(entityAABB.intersects(blockAABB))
            return false;
    }

    return true;
}
void AutoCrystal::generatePlacement(Actor* actor) {
    placeList.clear();

    const Vec3 targetPos = actor->getHumanPos();
    const BlockPos center((int)targetPos.x, (int)targetPos.y, (int)targetPos.z);
    int radius = (int)proximity;

    for(int x = -radius; x <= radius; x++) {
        for(int y = -2; y <= 2; y++) {
            for(int z = -radius; z <= radius; z++) {
                BlockPos blockPos = BlockPos(center.x + x, center.y + y, center.z + z);
                Vec3<float> blockPosFloat(blockPos.x + 0.5f, blockPos.y + 1.0f, blockPos.z + 0.5f);
                float dx = (blockPos.x + 0.5f) - targetPos.x;
                float dz = (blockPos.z + 0.5f) - targetPos.z;
                float distance = sqrtf(dx * dx + dz * dz);

                if(distance > placeRange || distance < 1.0f)
                    continue;

                if(isPlaceValid(blockPos, actor)) {
                    CrystalPlace placement(actor, blockPos);
                    placement.targetDamage = calculateDamage(blockPos, actor);
                    placement.localDamage = calculateDamage(blockPos,GI::getLocalPlayer());

                    if(placement.localDamage <= localDamage) {
                        placeList.emplace_back(placement);
                    }
                }
            }
        }
    }
    std::sort(
        placeList.begin(), placeList.end(), [&](const CrystalPlace& a, const CrystalPlace& b) {
            float diffA = a.targetDamage - a.localDamage;
            float diffB = b.targetDamage - b.localDamage;

            if(diffA != diffB)
                return diffA > diffB;

            float distA = Vec3<float>(a.blockPos.x + 0.5f, a.blockPos.y + 1.0f, a.blockPos.z + 0.5f)
                              .dist(targetPos);
            float distB = Vec3<float>(b.blockPos.x + 0.5f, b.blockPos.y + 1.0f, b.blockPos.z + 0.5f)
                              .dist(targetPos);
            if(distA != distB)
                return distA < distB;

            Vec3<float> eyePos = GI::getLocalPlayer()->getEyePos();
            float selfDistA =
                Vec3<float>(a.blockPos.x + 0.5f, a.blockPos.y + 1.0f, a.blockPos.z + 0.5f)
                    .dist(eyePos);
            float selfDistB =
                Vec3<float>(b.blockPos.x + 0.5f, b.blockPos.y + 1.0f, b.blockPos.z + 0.5f)
                    .dist(eyePos);
            return selfDistA < selfDistB;
        });
}
static constexpr int END_CRYSTAL_ID = 816;

int getCrystal() {
    auto* localPlayer = GI::getLocalPlayer();
    if (!localPlayer) return -1;  // 添加空指针检查
    
    auto* plrInv = localPlayer->getsupplies();
    if (!plrInv) return -1;  // 添加空指针检查
    
    auto* inv = plrInv->container;
    if (!inv) return -1;  // 添加空指针检查

    for(auto i = 0; i < 9; i++) {
        auto* itemStack = inv->getItem(i);
        if (!itemStack) continue;  // 添加空指针检查
        
        if(itemStack->valid && itemStack->mItem && itemStack->mItem->mItemId == END_CRYSTAL_ID) {
            return i;
        }
    }
    return plrInv->mSelectedSlot;
}
void AutoCrystal::getCrystals(Actor* actor) {
    for(Actor* entity : entityList) {
        if(entity == nullptr)
            continue;
        
        auto* actorTypeComp = entity->getActorTypeComponent();
        if (!actorTypeComp) continue;  // 添加空指针检查
        
        if(actorTypeComp->mType != ActorType::EnderCrystal)
            continue;
            
        auto* localPlayer = GI::getLocalPlayer();
        if (!localPlayer) continue;  // 添加空指针检查
        
        if(entity->getEyePos().dist(localPlayer->getEyePos()) > breakRange)
            continue;
            
        const CrystalBreak breakment(actor, entity);

        if(selfTest && actor == localPlayer) {
            auto* runtimeComp = entity->getRuntimeIDComponent();
            if (runtimeComp) {  // 添加空指针检查
                highestId = runtimeComp->mRuntimeID;
                breakList.emplace_back(breakment);
            }
        }
        else {
            if(breakment.targetDamage >= breakEnemyDamage &&
               breakment.localDamage <= breakLocalDamage) {
                auto* runtimeComp = entity->getRuntimeIDComponent();
                if (runtimeComp) {  // 添加空指针检查
                    highestId = runtimeComp->mRuntimeID;
                    breakList.emplace_back(breakment);
                }
            }
        }
    }
    if(!breakList.empty())
        std::sort(breakList.begin(), breakList.end(), sortCrystal);
}

void AutoCrystal::placeCrystal(GameMode* gm) {
    if(GI::getLocalPlayer() == nullptr)
        return;
    if(placeList.empty())
        return;


    int placed = 0;
    if(iPlaceDelay >= placeDelay) {
        for(const CrystalPlace& place : placeList) {
            if(switchMode >= 1) {
                int crystalSlot = getCrystal();
                if(crystalSlot >= 0 && InvUtil::getSelectedSlot() != crystalSlot) {
                    InvUtil::switchSlot(crystalSlot);
                }
            }

            gm->buildBlock(place.blockPos, 0, false);
            if(++placed >= wasteAmount)
                break;
        }
        iPlaceDelay = 0;
    } else
        iPlaceDelay++;
}
void AutoCrystal::breakCrystal(GameMode* gm) {
    if(breakList.empty())
        return;
    int breakInterval = std::max(1, breakDelay);

    if(iBreakDelay >= breakInterval) {
        Actor* crystalToBreak = breakList[0].crystal;
        if(crystalToBreak != nullptr) {
            if(predict) {
                highestId = crystalToBreak->getRuntimeIDComponent()->mRuntimeID;
            }

            int currentSlot = -1;
            if(switchMode == 2) {
                currentSlot = InvUtil::getSelectedSlot();
            }

            gm->attack(crystalToBreak);

            if(switchMode == 2 && currentSlot != -1) {
                InvUtil::switchSlot(currentSlot);
            }

            iBreakDelay = 0;
        }
    } else {
        iBreakDelay++;
    }
}

void AutoCrystal::breakIdPredictCrystal(GameMode* gm) {
    if(!abreak)
        return;
    if(placeList.empty())
        return;
    if(sendDelayTick >= sendDelay) {
        shouldChange = true;
        for(int i = 0; i < predictPacket; i++) {
            gm->attack(placeList[0].actor);
            highestId++;
        }
        highestId -= predictPacket;
        shouldChange = false;
        sendDelayTick = 0;
    } else {
        sendDelayTick++;
    }
}
void AutoCrystal::onNormalTick(LocalPlayer* localPlayer) {
    if (!localPlayer) return;  // 添加空指针检查
    
    targetList.clear();
    entityList.clear();
    placeList.clear();
    breakList.clear();
    
    if(eatstop && localPlayer->getItemUseDuration() > 0)
        return;
        
    auto* level = localPlayer->getlevel();
    if (!level) return;  // 添加空指针检查
    
    for(Actor* actor : level->getRuntimeActorList()) {
        if(!actor)
            continue;
        entityList.push_back(actor);
        if(!TargetUtil::isTargetValid(actor,false))
            continue;
            
        if(actor->getEyePos().dist(localPlayer->getEyePos()) > targetRange)
            continue;
        targetList.push_back(actor);
    }

    if(selfTest) {
        targetList.clear();  // Clear other targets in selfTest mode to focus only on self
        targetList.push_back(GI::getLocalPlayer());
    }

    if(targetList.empty())
        return;
    std::sort(targetList.begin(), targetList.end(), TargetUtil::sortByDist);
    generatePlacement(targetList[0]);
    getCrystals(targetList[0]);

    const int oldSlot = InvUtil::getSelectedSlot();

    int weaponSlot = oldSlot;  // Default to current slot if no better weapon found
    auto* plrInv = GI::getLocalPlayer()->getsupplies();
    auto* inv = plrInv->container;

 
    if(place) {
        auto* gamemode = localPlayer->gamemode;
        if (gamemode) {  // 添加空指针检查
            if(switchMode >= 1)
               InvUtil::switchSlot(getCrystal());
            placeCrystal(gamemode);
            if(switchMode == 2)
                InvUtil::switchSlot(oldSlot);
        }
    }

    if(abreak) {
        auto* gamemode = localPlayer->gamemode;
        if (gamemode) {  // 添加空指针检查
            if(predict) {
                breakIdPredictCrystal(gamemode);
            } else {
                if(switchMode >= 1)
                    InvUtil::switchSlot(weaponSlot);

                breakCrystal(gamemode);

                if(switchMode == 2)
                    InvUtil::switchSlot(oldSlot);
            }
        }
    }
}
void AutoCrystal::onSendPacket(Packet* packet) {
    if(GI::getLocalPlayer() == nullptr)
        return;

    if(rotate && !placeList.empty()) {
        const Vec2<float>& angle =
            GI::getLocalPlayer()->getEyePos().CalcAngle(placeList[0].blockPos.toFloat());
        if(packet->getId() == PacketID::PlayerAuthInput) {
            PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;
            authPkt->mRot = angle;
            authPkt->mYHeadRot = angle.y;
        }
    }

    if(predict && shouldChange) {
        if(packet->getId() == PacketID::InventoryTransaction) {
            InventoryTransactionPacket* invPkt = (InventoryTransactionPacket*)packet;
            ComplexInventoryTransaction* invComplex = invPkt->mTransaction.get();
            if(invComplex->type == ComplexInventoryTransaction::Type::ItemUseOnEntityTransaction) {
                *(int*)((uintptr_t)(invComplex) + 0x68) = highestId;
            }
        }
    }
}
/*
void AutoCrystal::onD2DRender() {
    if(renderType == 0 || placeList.empty())
        return;

    const int maxRender = std::min((int)placeList.size(), wasteAmount);
    for(int i = 0; i < maxRender; i++) {
        const CrystalPlace& place = placeList[i];
        AABB blockAABB;

        if(renderType == 1) {  // Box
            blockAABB.lower = Vec3<float>(place.blockPos.x, place.blockPos.y, place.blockPos.z);
            blockAABB.upper = blockAABB.lower.add(Vec3<float>(1.f, 1.f, 1.f));
        } else if(renderType == 2) {  // Flat (on top of block)
            blockAABB.lower =
                Vec3<float>(place.blockPos.x, place.blockPos.y + 1.f, place.blockPos.z);
            blockAABB.upper = blockAABB.lower.add(Vec3<float>(1.f, 0.f, 1.f));
        }

        float pulseAlpha = 1.f;
        if(i == 0) {
            static float pulseTimer = 0.f;
            pulseTimer += D2D::deltaTime;
            float time = pulseTimer * 2.f;                   // control speed
            pulseAlpha = 0.6f + (sinf(time) * 0.4f + 0.4f);  // value in approx [0.2,1.0]
        }

        UIColor fillClr = renderColor;
        fillClr.a = static_cast<uint8_t>(fillClr.a * pulseAlpha);
        UIColor outlineClr = lineColor;
        outlineClr.a = static_cast<uint8_t>(outlineClr.a * pulseAlpha);

        D2D::drawBox(blockAABB, fillClr, outlineClr, 1.5f, true, true);
    }

    for(auto it = fadeList.begin(); it != fadeList.end();) {
        it->fadeTimer -= D2D::deltaTime;
        if(it->fadeTimer <= 0.f)
            it = fadeList.erase(it);
        else
            ++it;
    }

    if(!placeList.empty()) {
        const auto& best = placeList[0];
        Vec3<float> posFloat(best.blockPos.x, best.blockPos.y, best.blockPos.z);
        bool found = false;
        for(auto& f : fadeList) {
            if(f.lastPos == posFloat) {
                f.fadeTimer = fadeDur;  // reset timer when we are still rendering this block
                found = true;
                break;
            }
        }
        if(!found) {
            CrystalFadeStruct cs;
            cs.lastPos = posFloat;
            cs.fadeTimer = fadeDur;
            cs.fadeDuration = fadeDur;
            fadeList.emplace_back(cs);
        }
    }

    for(const auto& f : fadeList) {
        bool isCurrent = false;
        for(int i = 0; i < maxRender; ++i) {
            if(Vec3<float>(placeList[i].blockPos.x, placeList[i].blockPos.y,
                           placeList[i].blockPos.z) == f.lastPos) {
                isCurrent = true;
                break;
            }
        }
        if(isCurrent)
            continue;

        float alphaFactor = f.fadeTimer / f.fadeDuration;
        UIColor fadeClr = renderColor;
        fadeClr.a = static_cast<uint8_t>(fadeClr.a * alphaFactor);
        UIColor fadeOutline = lineColor;
        fadeOutline.a = static_cast<uint8_t>(fadeOutline.a * alphaFactor);

        BlockPos bp((int)f.lastPos.x, (int)f.lastPos.y, (int)f.lastPos.z);
        AABB fadeAABB;
        if(renderType == 1) {
            fadeAABB.lower = Vec3<float>(bp.x, bp.y, bp.z);
            fadeAABB.upper = fadeAABB.lower.add(Vec3<float>(1.f, 1.f, 1.f));
        } else if(renderType == 2) {
            fadeAABB.lower = Vec3<float>(bp.x, bp.y + 1.f, bp.z);
            fadeAABB.upper = fadeAABB.lower.add(Vec3<float>(1.f, 0.f, 1.f));
        }

        D2D::drawBox(fadeAABB, fadeClr, fadeOutline, 1.f, true, true);
    }
}
*/
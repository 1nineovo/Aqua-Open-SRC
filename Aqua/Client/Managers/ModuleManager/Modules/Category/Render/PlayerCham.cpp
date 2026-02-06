#include "PlayerCham.h"


#include "../../../../../../Utils/ColorUtil.h"
#include "../../../../../../Utils/Logger.h"
#include "../../../../../../Utils/TimerUtil.h"
#include "../../../ModuleManager.h"
#include <DrawUtil.h>

PlayerCham::PlayerCham() : Module("PlayerChams", "Chams of target.", Category::RENDER) {
    registerSetting(new ColorSetting("Color", "Primary color", &col, col, false));
    registerSetting(new ColorSetting("Line Color", "Line color", &linecol, linecol, false));
    registerSetting(new BoolSetting("Dancing", "Do ur chams and player dance", &isdancing, false));
}

void PlayerCham::onLevelRender() {
    LocalPlayer* lp = GI::getLocalPlayer();
    if(!lp || !lp->level)
        return;
    isFirstPerson = (GC::perspective == 0);

    for(auto& player : lp->level->getRuntimeActorList()) {
        if(!player)
            continue;

        if(player->getActorTypeComponent()->mType != ActorType::Player)
            continue;

        AABBShapeComponent* aabbShape = player->getAABBShapeComponent();
        if(!aabbShape)
            continue;
        if(!player->isAlive())
            continue;
        isLocal = (player == lp);

        if(isLocal && isFirstPerson)
            continue;

        Vec3<float> vel = player->getStateVectorComponent()->mPosOld;
        Vec3<float> vel2 = player->getStateVectorComponent()->mPos;

        bool isSlowMoving = false;
        bool isFastMoving = false;
        bool isSneaking = false;
        bool isjumping = false;

        float yawDeg = player->getActorRotationComponent()->mOldYaw;
        float pitchDeg = player->getActorRotationComponent()->mOldPitch;
        float yawRad = yawDeg * PI / 180.f;
        float pitchRad = pitchDeg * PI / 180.f;
        float cy = cosf(yawRad), sy = sinf(yawRad);

        float speedSq = vel.dist(vel2);
        isSlowMoving = speedSq > 0.0f && speedSq < 0.25f;
        isFastMoving = speedSq > 0.25f;
        isSneaking = false;
        isjumping = player->getFallDisntance() > 0;

        static float animTime = 0.f;
        static float walkTime = 0.f;

        if(isdancing) {
            animTime += DrawUtil::deltaTime;
        } else {
            animTime = 0.f;
        }

        float bodySneakingPitchDeg = isSneaking ? -30.f : 0.f;
        float bodySneakingPitchRad = bodySneakingPitchDeg * PI / 180.f;

        Vec3<float> eye = player->getEyePos();
        Vec3<float> headCenter = eye;

        DrawUtil::drawBox3dFilledRotP(headCenter, headHalf, pitchRad, yawRad, col, linecol);
        Vec3<float> bodyCenter = headCenter.add(Vec3<float>(0.f, -0.55f, 0.f));

        DrawUtil::drawBox3dFilledRotP_TopCenter(bodyCenter, bodyHalf, bodySneakingPitchRad, yawRad, col,
                                           linecol);

        bool moving = isSlowMoving || isFastMoving;

        if(moving) {
            float speedMul = isFastMoving ? 2.0f : 1.0f;
            walkTime += DrawUtil::deltaTime * 6.0f * speedMul;
        } else {
            walkTime = fmodf(walkTime, 2 * PI);
            float diff = sinf(walkTime);
            if(fabsf(diff) < 0.02f)
                walkTime = 0.f;
            else
                walkTime += (diff > 0 ? -1 : 1) * DrawUtil::deltaTime * 3.f;
        }

        float swingAmpDeg = isFastMoving ? 45.f : 25.f;
        if(isSneaking)
            swingAmpDeg *= 0.4f;
        float swingRad = sinf(walkTime) * (swingAmpDeg * PI / 180.f);

        auto toWorld = [&](const Vec3<float>& local) {
            return Vec3<float>(local.x * cy - local.z * sy, local.y, local.x * sy + local.z * cy);
        };

        Vec3<float> leftArmCenter = bodyCenter.add(toWorld(localLeftOffset1));
        Vec3<float> rightArmCenter = bodyCenter.add(toWorld(localRightOffset1));
        Vec3<float> leftLegCenter = bodyCenter.add(toWorld(localLeftLegOffset));
        Vec3<float> rightLegCenter = bodyCenter.add(toWorld(localRighLegOffset));

        float leftArmPitch = swingRad;
        float rightArmPitch = -swingRad;

        if(isdancing) {
            const float w = 2.f * PI * 0.8f;
            float offset = 80.f * PI / 180.f * sinf(animTime * w);

            const float edgeThresh = 75.f * PI / 180.f;
            const float shakeDur = 0.3f;
            const float shakeFreq = 12.f;
            const float shakeAmp = 20.f * PI / 180.f;

            static float leftEdgeTimer = 0.f;
            static float rightEdgeTimer = 0.f;

            if(fabsf(offset) > edgeThresh) {
                if(offset > 0)
                    leftEdgeTimer += DrawUtil::deltaTime;
                else
                    rightEdgeTimer += DrawUtil::deltaTime;
            } else {
                leftEdgeTimer = rightEdgeTimer = 0.f;
            }

            auto calcShake = [&](float timer) {
                if(timer == 0.f || timer > shakeDur)
                    return 0.f;
                return shakeAmp * sinf(timer * shakeFreq * 2.f * PI);
            };

            float leftShake = calcShake(leftEdgeTimer);
            float rightShake = calcShake(rightEdgeTimer);

            leftArmPitch = PI / 2 + offset + leftShake;
            rightArmPitch = PI / 2 - offset + rightShake;
        }

        if(isSneaking) {
            leftArmPitch += bodySneakingPitchRad;
            rightArmPitch += bodySneakingPitchRad;
        }

        DrawUtil::drawBox3dFilledRotP_BottomCenter(leftArmCenter, armHalf, leftArmPitch, yawRad, col,
                                              linecol);
        DrawUtil::drawBox3dFilledRotP_BottomCenter(rightArmCenter, armHalf, rightArmPitch, yawRad, col,
                                              linecol);

        float leftLegPitch = -swingRad;
        float rightLegPitch = swingRad;

        if(isdancing) {
            auto armOffset = [&](float pitch) { return (pitch - PI / 2); };
            leftLegPitch = -armOffset(leftArmPitch) * 0.5f;
            rightLegPitch = -armOffset(rightArmPitch) * 0.5f;
        }

        DrawUtil::drawBox3dFilledRotP_BottomCenter(leftLegCenter, legHalf, leftLegPitch, yawRad, col,
                                              linecol);
        DrawUtil::drawBox3dFilledRotP_BottomCenter(rightLegCenter, legHalf, rightLegPitch, yawRad, col,
                                              linecol);
    }
}
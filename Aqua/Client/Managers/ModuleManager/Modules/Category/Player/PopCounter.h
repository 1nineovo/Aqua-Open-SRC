#pragma once
#include <AnimationUtil.h>
#include <ColorUtil.h>
#include <DrawUtil.h>

#include <chrono>
#include <vector>

#include "../../ModuleBase/Module.h"

struct PopAnimationData {
    Vec3<float> startPos;    // ��ʼλ��
    Vec3<float> currentPos;  // ��ǰλ��
    float startTime;         // ��ʼʱ��
    float duration;          // ��������ʱ��
    UIColor color;           // �����ɫ
    UIColor lineColor;       // �߿���ɫ
    float pitch;             // ��Ҹ�����
    float yaw;               // ���ƫ����
    bool isFinished;         // �Ƿ����

    PopAnimationData(Vec3<float> pos, float p, float y) {
        startPos = pos;
        currentPos = pos;
        startTime = GetTickCount64() / 1000.0f;
        duration = 3.0f;  // 3�붯��
        pitch = p;
        yaw = y;
        isFinished = false;

        generateRandomColor();
    }

   private:
    void generateRandomColor() {
        int colorType = rand() % 6;
        switch(colorType) {
            case 0:
                color = UIColor(255, rand() % 128 + 127, rand() % 128, 200);
                break;  // ��ɫϵ
            case 1:
                color = UIColor(rand() % 128, 255, rand() % 128 + 127, 200);
                break;  // ��ɫϵ
            case 2:
                color = UIColor(rand() % 128, rand() % 128 + 127, 255, 200);
                break;  // ��ɫϵ
            case 3:
                color = UIColor(255, 255, rand() % 128, 200);
                break;  // ��ɫϵ
            case 4:
                color = UIColor(255, rand() % 128, 255, 200);
                break;  // Ʒ��ϵ
            case 5:
                color = UIColor(rand() % 128 + 127, 255, 255, 200);
                break;  // ��ɫϵ
        }
        lineColor = UIColor(color.r, color.g, color.b, 255);  // �߿���ȫ��͸��
    }
};

class PopCounter : public Module {
   private:
    bool sendchat = false;
    bool popcham = true;
    std::string names = "PopCounter";

    std::vector<PopAnimationData> popAnimations;

   public:
    PopCounter();
    void onNormalTick(LocalPlayer* localPlayer) override;
    void onLevelRender() override;
};
#pragma once
#include "../../ModuleBase/Module.h"
#include "../../ModuleBase/Settings/TextSetting.h"

class Interface : public Module {
   private:
 
    struct EffectInfo {
        std::string name;
        int level;
        int duration;   
        int totalDuration;  
        std::string timeLeft;
        bool isBeneficial;
        int effectId;
    };

    std::unordered_map<int, int> effectTotalDurations;

    Vec2<float> potionEffectPos = Vec2<float>(0, 50);
    float potionEffectSize = 1.0f;

    std::string getEffectName(uint32_t effectId) {
        switch(effectId) {
            case 1:
                return "Speed";
            case 2:
                return "Slowness";
            case 3:
                return "Haste";
            case 4:
                return "Mining Fatigue";
            case 5:
                return "Strength";
            case 6:
                return "Instant Health";
            case 7:
                return "Instant Damage";
            case 8:
                return "Jump Boost";
            case 9:
                return "Nausea";
            case 10:
                return "Regeneration";
            case 11:
                return "Resistance";
            case 12:
                return "Fire Resistance";
            case 13:
                return "Water Breathing";
            case 14:
                return "Invisibility";
            case 15:
                return "Blindness";
            case 16:
                return "Night Vision";
            case 17:
                return "Hunger";
            case 18:
                return "Weakness";
            case 19:
                return "Poison";
            case 20:
                return "Wither";
            case 21:
                return "Health Boost";
            case 22:
                return "Absorption";
            case 23:
                return "Saturation";
            case 24:
                return "Glowing";
            case 25:
                return "Levitation";
            case 26:
                return "Luck";
            case 27:
                return "Bad Luck";
            case 28:
                return "Slow Falling";
            case 29:
                return "Conduit Power";
            case 30:
                return "Dolphins Grace";
            case 31:
                return "Bad Omen";
            case 32:
                return "Hero of Village";
            case 33:
                return "Darkness";
            case 34:
                return "Trial Omen";
            case 35:
                return "Wind Charged";
            case 36:
                return "Weaving";
            default:
                return "Unknown Effect";
        }
    }

    std::string getRomanNumeral(int number) {
        if(number <= 0 || number > 20)
            return std::to_string(number);

        const std::string romanNumerals[] = {"",    "I",    "II",  "III",  "IV",    "V",   "VI",
                                             "VII", "VIII", "IX",  "X",    "XI",    "XII", "XIII",
                                             "XIV", "XV",   "XVI", "XVII", "XVIII", "XIX", "XX"};

        return romanNumerals[number];
    }

    std::string getEffectTimeLeftStr(MobEffectInstance* mEffectInstance) {
        uint32_t timeLeft = mEffectInstance->mDuration;
        uint32_t timeReal = (uint32_t)(timeLeft / 20);
        std::string m = std::to_string(timeReal / 60);
        std::string s;
        if(timeReal % 60 < 10)
            s += "0";
        s += std::to_string(timeReal % 60);
        return m + ":" + s;
    }

    bool isBeneficialEffect(uint32_t effectId) {
        switch(effectId) {
            case 1:
            case 3:
            case 5:
            case 6:
            case 8:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 16:
            case 21:
            case 22:
            case 23:
            case 26:
            case 28:
            case 29:
            case 30:
            case 32:
                return true;
            default:
                return false;
        }
    }

    UIColor getEffectColor(uint32_t effectId) {
        switch(effectId) {
            case 1:
                return UIColor(124, 175, 198, 255);  // Speed - 浅蓝
            case 2:
                return UIColor(90, 108, 129, 255);  // Slowness - 灰蓝
            case 3:
                return UIColor(217, 192, 67, 255);  // Haste - 金黄
            case 4:
                return UIColor(74, 66, 23, 255);  // Mining Fatigue - 暗黄
            case 5:
                return UIColor(147, 36, 35, 255);  // Strength - 红色
            case 6:
                return UIColor(248, 36, 35, 255);  // Instant Health - 亮红
            case 8:
                return UIColor(34, 255, 76, 255);  // Jump Boost - 绿色
            case 10:
                return UIColor(205, 92, 171, 255);  // Regeneration - 粉色
            case 11:
                return UIColor(153, 69, 58, 255);  // Resistance - 棕色
            case 12:
                return UIColor(228, 154, 58, 255);  // Fire Resistance - 橙色
            case 13:
                return UIColor(46, 82, 153, 255);  // Water Breathing - 蓝色
            case 14:
                return UIColor(127, 131, 146, 255);  // Invisibility - 灰色
            case 16:
                return UIColor(31, 31, 161, 255);  // Night Vision - 深蓝
            case 19:
                return UIColor(78, 147, 49, 255);  // Poison - 绿色
            case 20:
                return UIColor(53, 42, 39, 255);  // Wither - 黑色
            default:
                return UIColor(160, 160, 160, 255); 
        }
    }

    void renderPotionEffects(const Vec2<float>& windowsSize, Actor* localPlayer,
                             ImDrawList* drawList);

   public:
    struct HotBar {  
        Vec4<float> pos;
    };
    std::vector<HotBar> hotbars;

    Vec4<float> sHotbarRectPos;
    float itemSize = 1.0f;
    bool hotbar = false;
    bool effect = false;
    bool keystrokes = false;
    int colorType = 0;
    UIColor mainColor = UIColor(115, 135, 255, 125);
    UIColor primaryColor = UIColor(115, 135, 255, 125);
    UIColor secondColor = UIColor(115, 135, 255, 125);

    float saturation = 1.f;
    int seperation = 50;
    float seconds = 6.f;
    float brightness = 1.f;

   public:
    Interface();
    void onEnable() override;
    void onDisable() override;
    void onD2DRender() override;
    void onImGuiRender(ImDrawList* drawList) override;
    std::string getDirectionString(float yaw);
    void renderHudInfo(ImDrawList* drawList);
    void onNormalTick(LocalPlayer* lp) override;
    void renderHotbar();
    void renderKeystrokes(ImDrawList* drawlist);

    bool showPing = false;
    bool showFps = false;
    bool showXyz = false;
    bool showDirection = false;

    
    UIColor getColor(int index = 0) {
        if(colorType == 1)
            return ColorUtil::getWaveColor(primaryColor, secondColor, index);
        else if(colorType == 2)
            return ColorUtil::getRainbowColor(seconds, saturation, brightness, index);
        return mainColor;
    }
};
#pragma once
#include <string>
#include <stdint.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <algorithm>
#include <vector>
#include <cmath>

#include "../SDK/Core/mce.h"

struct UIColor {
    union {
        struct {
            uint8_t r, g, b, a;
        };
        int arr[4];
    };

    // ... (你的 FromHSV 保持不变) ...
    static UIColor FromHSV(float h, float s, float v, float a = 1.0f) {
        // ... 代码省略 ...
        return UIColor(/*...*/);  // 这里为了节省篇幅省略，保持你原有的即可
    }

    UIColor(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255, uint8_t alpha = 255) {
        this->r = red;
        this->g = green;
        this->b = blue;
        this->a = alpha;
    }

    UIColor(const mce::Color& color) {
        this->r = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
        this->g = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
        this->b = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
        this->a = static_cast<uint8_t>(std::clamp(color.a * 255.0f, 0.0f, 255.0f));
    }

    bool operator==(const UIColor& other) const {
        return (r == other.r && g == other.g && b == other.b && a == other.a);
    }

    D2D1_COLOR_F toD2D1Color() const {
        return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    mce::Color toMCColor() const {
        return mce::Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    ImColor toImColor(int alpha = -1) const {
        int finalAlpha = (alpha < 0) ? this->a : std::clamp(alpha, 0, 255);
        return ImColor(this->r, this->g, this->b, static_cast<uint8_t>(finalAlpha));
    }
};

class ColorUtil {
   public:
    inline static unsigned __int64 getCurrentMs() {
        FILETIME f;
        GetSystemTimeAsFileTime(&f);
        (unsigned long long)f.dwHighDateTime;
        unsigned __int64 nano =
            ((unsigned __int64)f.dwHighDateTime << 32LL) + (unsigned __int64)f.dwLowDateTime;
        return (nano - 116444736000000000LL) / 10000;
    }
    static uint32_t ColorToUInt(const UIColor& color) {
        return (static_cast<uint32_t>(color.a) << 24) | (static_cast<uint32_t>(color.r) << 16) |
               (static_cast<uint32_t>(color.g) << 8) | static_cast<uint32_t>(color.b);
    }
    static  UIColor HexStringToColor(const std::string& hexString) {
        std::string hex = hexString;
        if(hex.substr(0, 2) == "0x") {
            hex = hex.substr(2);
        }

        if(hex.length() != 8) {
            return UIColor(0, 0, 0, 255);
        }

        try {
            uint32_t colorValue = std::stoul(hex, nullptr, 16);
            return UIColor(static_cast<uint8_t>((colorValue >> 24) & 0xFF),
                           static_cast<uint8_t>((colorValue >> 16) & 0xFF),
                           static_cast<uint8_t>((colorValue >> 8) & 0xFF),
                           static_cast<uint8_t>(colorValue & 0xFF));
        } catch(...) {
            return UIColor(0, 0, 0, 255);
        }
    }
    static std::string ColorToHexString(const UIColor& color) {
        std::stringstream ss;
        ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.r)
           << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.g) << std::hex
           << std::setw(2) << std::setfill('0') << static_cast<int>(color.b) << std::hex
           << std::setw(2) << std::setfill('0') << static_cast<int>(color.a);
        return ss.str();
    }

    static void ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s,
                                     float& out_v) {
        float K = 0.f;

        // 排序逻辑，为了减少 if-else 分支，使用类似 GLSL 的混合算法
        if(g < b) {
            std::swap(g, b);
            K = -1.f;
        }
        if(r < g) {
            std::swap(r, g);
            K = -2.f / 6.f - K;
        }

        float chroma = r - (g < b ? g : b);

        // Hue calculation
        out_h = fabsf(K + (g - b) / (6.f * chroma + 1e-20f));

        // Saturation calculation
        out_v = r;
        out_s = chroma / (r + 1e-20f);
    }

     static void ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g,
                                         float& out_b) {
        if(s == 0.0f) {
            out_r = out_g = out_b = v;
            return;
        }

        h = fmodf(h, 1.0f) / (60.0f / 360.0f);
        int i = (int)h;
        float f = h - (float)i;
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch(i) {
            case 0:
                out_r = v;
                out_g = t;
                out_b = p;
                break;
            case 1:
                out_r = q;
                out_g = v;
                out_b = p;
                break;
            case 2:
                out_r = p;
                out_g = v;
                out_b = t;
                break;
            case 3:
                out_r = p;
                out_g = q;
                out_b = v;
                break;
            case 4:
                out_r = t;
                out_g = p;
                out_b = v;
                break;
            case 5:
            default:
                out_r = v;
                out_g = p;
                out_b = q;
                break;
        }
    }
    static UIColor lerp(const UIColor& start, const UIColor& end, float t);
    static UIColor lerpHSV(const UIColor& start, const UIColor& end, float t);

    static UIColor getRainbowColor(float seconds, float saturation, float brightness, long index) {
        float currentHue =
            (((getCurrentMs() + index) % (int)(seconds * 1000)) / (float)(seconds * 1000));
        float red, green, blue = 0.0f;
        ColorConvertHSVtoRGB(currentHue, saturation, brightness, red, green, blue);

        return UIColor((int)(red * 255.f), (int)(green * 255.f), (int)(blue * 255.f));
    }
    static UIColor getWaveColor(const UIColor& startColor, const UIColor& endColor, int index) {
        double offset = ((getCurrentMs() - index) / 8) / (double)120;
        double aids123 = ((getCurrentMs() - index) % 1000 / 1000.000);
        int aids1234 = ((getCurrentMs() - index) % 2000 / 2000.000) * 2;
        aids123 = aids1234 % 2 == 0 ? aids123 : 1 - aids123;
        double inverse_percent = 1 - aids123;
        int redPart = (int)(startColor.r * inverse_percent + endColor.r * aids123);
        int greenPart = (int)(startColor.g * inverse_percent + endColor.g * aids123);
        int bluePart = (int)(startColor.b * inverse_percent + endColor.b * aids123);
        return UIColor(redPart, greenPart, bluePart);
    }
    static UIColor getBreathingColor(const UIColor& color, float time, float speed = 1.0f);
    static UIColor getPulseColor(const UIColor& color, float time, float intensity = 1.0f);

    static UIColor getOceanWaveColor(float time, float phase = 0.0f);
    static UIColor getAquaGradient(float progress, float alpha = 1.0f);
    static UIColor getCrystalShine(float time, float phase = 0.0f);
    static UIColor getLiquidFlow(float time, float phase = 0.0f);
    static UIColor getPlasmaEffect(float time, float phase = 0.0f);
    static UIColor getSweepGradient(float progress, float baseHue = 200.0f);
    static UIColor getGlowPulse(const UIColor& baseColor, float time, float intensity = 1.0f);
    static UIColor getEnergyFlow(float time, float phase = 0.0f);
    static UIColor getNeonGlow(float time, float hue = 200.0f, float intensity = 1.0f);

    static UIColor getMultiGradient(const std::vector<UIColor>& colors, float progress);
    static UIColor getAquaThemeColor(int variant, float alpha = 1.0f);

    struct GradientStop {
        float position;
        UIColor color;
    };

    static ID2D1LinearGradientBrush* createLinearGradientBrush(
        ID2D1RenderTarget* renderTarget,
                                                        const std::vector<GradientStop>& stops,
                                                        const D2D1_POINT_2F& startPoint,
                                                        const D2D1_POINT_2F& endPoint);

    static ID2D1RadialGradientBrush* createRadialGradientBrush(
        ID2D1RenderTarget* renderTarget,
                                                        const std::vector<GradientStop>& stops,
                                                        const D2D1_POINT_2F& center, float radiusX,
                                                        float radiusY);

    static std::vector<GradientStop> getAquaGradientStops(float alpha = 1.0f);
    static std::vector<GradientStop> getOceanGradientStops(float time, float alpha = 1.0f);
    static std::vector<GradientStop> getSunsetGradientStops(float alpha = 1.0f);
    static std::vector<GradientStop> getNeonGradientStops(float time, float alpha = 1.0f);
};
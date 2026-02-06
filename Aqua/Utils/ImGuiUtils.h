#pragma once
#include <string>
#include "ColorUtil.h"
#include "Math.h"
#include "../Libs/ImGui/imgui.h"

class ImGuiUtils {
private:
	static inline ImDrawList* drawlist = nullptr;
	static inline float fontSize = 25.f;
	static inline float shadowOffset = 1.f;
public:
	static void setDrawList(ImDrawList* d);
	static void drawText(Vec2<float> textPos, std::string textStr, UIColor color, float textSize, bool glow = false, float thick_ness = 0.f);
	static void setFontSize(float newSize) { fontSize = newSize; };
	static void setShadowOffset(float offset) { shadowOffset = offset; }
	static float getTextWidth(std::string textStr, float textSize = 1.f);
	static float getTextHeight(float textSize = 1.f);
	static void drawRectangle(const Vec4<float>& rectPos, UIColor color, float lineWidth = 1.0f, float rounding = 0.0f, ImDrawFlags flags = 0);
	static void drawRectangle(Vec2<float> pMin, Vec2<float> pMax, UIColor color, float lineWidth = 1.0f, float rounding = 0.0f, ImDrawFlags flags = 0);
	static void drawRectangleShadow(const Vec4<float>& rectPos, UIColor color, float shadow_thickness, const ImVec2& shadow_offset, float rounding, ImDrawFlags flags  = 0);
	static void fillRectangle(Vec4<float> rectPos, UIColor color, float rounding = 0.0f, ImDrawFlags flags = 0);
	static void fillRectangle(Vec2<float> pMin, Vec2<float> pMax, UIColor color, float rounding = 0.0f, ImDrawFlags flags = 0);
	static void drawBox(const AABB& blockAABB, UIColor color, UIColor lineColor, float lineWidth, bool fill, bool outline);
	static void drawBox(const Vec3<float>& blockPos, UIColor color, UIColor lineColor, float lineWidth, bool fill, bool outline);
	static bool worldToScreen(const Vec3<float>& pos, Vec2<float>& out);
	static void renderCircle(Vec2<float> pos, ImColor color, float radius = 1.f, float thickness = 1.f, int num_seg = 0);
	static void renderCircleFilled(Vec2<float> pos, ImColor color, float radius = 1.f, int num_seg = 0);
    static void renderCircleShadow(Vec2<float> pos, ImColor color, float radius = 1.f,
                                   float thickness = 5.f, bool fill = false, int num_seg = 12,
                                   ImVec2 offset = ImVec2(0, 0));
    static Vec2<float> WorldToScreen(const Vec3<float>& worldPos, const Vec2<float>& screenSize);
};
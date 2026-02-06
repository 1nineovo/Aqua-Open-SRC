#pragma once


#include <d2d1_3.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <d3d12.h>
#include <dwrite_3.h>
#include <dxgi1_4.h>

#include <string>

#include "../Utils/ColorUtil.h"
#include "../Utils/Maths.h"

namespace RenderUtil {


extern Vec2<float> mpos;

extern float deltaTime;

void NewFrame(IDXGISwapChain3* swapChain, ID3D11Device* d3d11Device, float fxdpi);
void EndFrame();
void Render();
void Clean();
void Flush();

Vec2<float> getWindowSize();
void drawText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
              float textSize = 1.f, bool storeTextLayout = true);
float getTextWidth(const std::string& textStr, float textSize = 1.f, bool storeTextLayout = true);
float getTextHeight(const std::string& textStr, float textSize = 1.f, bool storeTextLayout = true);
void drawLine(const Vec2<float>& startPos, const Vec2<float>& endPos, const UIColor& color,
              float width = 1.f);
void drawRectangle(const Vec4<float>& rect, const UIColor& color, float width = 1.f);
void fillRectangle(const Vec4<float>& rect, const UIColor& color);
void drawCircle(const Vec2<float>& centerPos, const UIColor& color, float radius,
                float width = 1.f);
void fillCircle(const Vec2<float>& centerPos, const UIColor& color, float radius);
void addBlur(const Vec4<float>& rect, float strength, bool flush = true);
void drawRoundedRectangle(const Vec4<float>& rect, const UIColor& color, float radius = 5.f,
                          float width = 1.f);
void fillRoundedRectangle(const Vec4<float>& rect, const UIColor& color, float radius = 5.f);
void fillRect(const Vec2<float>& pos, const Vec2<float>& size, const UIColor& color);
void drawTriangle(const Vec4<float>& rect, const UIColor& color, bool leftDirection, bool filled);
void drawPause(const Vec4<float>& rect, const UIColor& color);
Vec2<float> WorldToScreen(const Vec3<float>& worldPos, const Vec2<float>& screenSize);
void drawTextInWorld(const Vec3<float>& worldPos, const std::string& textStr, const UIColor& color,
                     float textSize = 1.f);

bool ScreenChange(std::string& from, std::string& to);

void addShadow(const Vec4<float>& rect, float strength, const UIColor& shadowColor, float rounding,
               const std::vector<Vec4<float>>& excludeRects);

void addDropShadow(const Vec4<float>& rect, float blurRadius, const UIColor& shadowColor,
                   const Vec2<float>& offset, float rounding);

} 

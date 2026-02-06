#include <Windows.h>
#include <wincodec.h>
#include <winrt/base.h>
#include <AnimationUtil.h>
#include <unordered_map>
#include <vector>
#include "../Client/Client.h"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/BGAnimation.h"
#include "../Client/Managers/ModuleManager/Modules/Category/Client/CustomFont.h"
#include "../SDK/GlobalInstance.h"
#include "../Utils/Logger.h"
#include "../Utils/NetworkUtil.h"
#include "../Utils/TimerUtil.h"
#include "RenderUtil.h"
#include "../SDK/Render/Matrix.h"
#include "../Client/Managers/HooksManager/Hooks/Network/GetAvgPingHook.h"
#include <SkinGameData.h>
#include <filesystem>
#include <fstream>
#include "../Utils/FileUtil.h"
#include <algorithm>
#include <set>
#include <thread>
float RenderUtil::deltaTime = 0.016f;
Vec2<float> RenderUtil::mpos = Vec2<float>(0.f, 0.f);

// d2d stuff
static ID2D1Factory3* d2dFactory = nullptr;
static IDWriteFactory* d2dWriteFactory = nullptr;
static ID2D1Device2* d2dDevice = nullptr;
static ID2D1DeviceContext2* d2dDeviceContext = nullptr;
static ID2D1Bitmap1* sourceBitmap = nullptr;
static ID2D1Effect* blurEffect = nullptr;
static ID2D1Bitmap1* shadowBitmap = nullptr;
static ID2D1Effect* shadowEffect = nullptr;
static ID2D1Effect* stencilEffect = nullptr;
// cache
static std::unordered_map<float, winrt::com_ptr<IDWriteTextFormat>> textFormatCache;
static std::unordered_map<uint64_t, winrt::com_ptr<IDWriteTextLayout>> textLayoutCache;
static std::unordered_map<uint32_t, winrt::com_ptr<ID2D1SolidColorBrush>> colorBrushCache;
static std::unordered_map<std::string, winrt::com_ptr<ID2D1Bitmap1>> skinBitmapCache;
static std::unordered_map<std::string, std::string> imageCacheMap;


// 添加失败URL追踪
static std::unordered_set<std::string> failedUrls;
static std::unordered_map<std::string, int> failureCount;
// temporary cache
static std::unordered_map<uint64_t, winrt::com_ptr<IDWriteTextLayout>> textLayoutTemporary;

static int currentD2DFontSize = 25;
static std::string currentD2DFont = "Microsoft YaHei UI";
static bool isFontItalic = false;

static bool initD2D = false;

static IWICImagingFactory* wicFactory = nullptr;
static std::unordered_map<std::string, winrt::com_ptr<ID2D1Bitmap1>> urlBitmapCache;

// Helper: ensure WIC factory
static void EnsureWICFactory() {
    if(!wicFactory) {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&wicFactory));
    }
}
template <typename T>
void SafeRelease(T*& ptr) {
    if(ptr != nullptr) {
        ptr->Release();
        ptr = nullptr;
    }
}

std::wstring to_wide(const std::string& str);
uint64_t getTextLayoutKey(const std::string& textStr, float textSize);
IDWriteTextFormat* getTextFormat(float textSize);
IDWriteTextLayout* getTextLayout(const std::string& textStr, float textSize,
                                 bool storeTextLayout = true);
ID2D1SolidColorBrush* getSolidColorBrush(const UIColor& color);

// Guard macro to early-return when D2D context is missing
#define D2D_CTX_GUARD()   \
    if(!d2dDeviceContext) \
        return;
#define D2D_CTX_GUARD_RET(defaultVal) \
    if(!d2dDeviceContext)             \
        return defaultVal;

void RenderUtil::NewFrame(IDXGISwapChain3* swapChain, ID3D11Device* d3d11Device, float fxdpi) {
    if(!initD2D) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &d2dFactory);

        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(d2dWriteFactory),
                            reinterpret_cast<IUnknown**>(&d2dWriteFactory));

        IDXGIDevice* dxgiDevice;
        d3d11Device->QueryInterface<IDXGIDevice>(&dxgiDevice);
        d2dFactory->CreateDevice(dxgiDevice, &d2dDevice);
        dxgiDevice->Release();

        d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dDeviceContext);
        // d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
        // &d2dDeviceContext);

        d2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &blurEffect);
        blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
        blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
                             D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY);

    
        d2dDeviceContext->CreateEffect(CLSID_D2D1Shadow, &shadowEffect);


        d2dDeviceContext->CreateEffect(CLSID_D2D1AlphaMask, &stencilEffect);


        IDXGISurface* dxgiBackBuffer = nullptr;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), fxdpi, fxdpi);
        d2dDeviceContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer, &bitmapProperties,
                                                      &sourceBitmap);
        dxgiBackBuffer->Release();

        d2dDeviceContext->SetTarget(sourceBitmap);

        initD2D = true;
    }

    d2dDeviceContext->BeginDraw();
}

void RenderUtil::EndFrame() {
    if(!initD2D)
        return;

    d2dDeviceContext->EndDraw();
    HRESULT hr = d2dDeviceContext->EndDraw();
    if(FAILED(hr)) {

        return;
    }

    if(!Client::isInitialized()) {
        return;
    }

    static CustomFont* customFontMod = nullptr;
    if(!customFontMod) {
        customFontMod = ModuleManager::getModule<CustomFont>();
    }

    if(customFontMod && Client::isInitialized()) {
        if((currentD2DFont != customFontMod->getSelectedFont()) ||
           (currentD2DFontSize != customFontMod->fontSize)
           //(isFontItalic != customFontMod->italic)
           ) {
            //currentD2DFont = customFontMod->getSelectedFont();
            currentD2DFontSize = customFontMod->fontSize;
            //isFontItalic = customFontMod->italic;
            textFormatCache.clear();
            textLayoutCache.clear();
        }
    }

    static float timeCounter = 0.0f;
    timeCounter += RenderUtil::deltaTime;
    if(timeCounter > 90.f) {
        if(textFormatCache.size() > 1000)
            textFormatCache.clear();

        if(textLayoutCache.size() > 500)
            textLayoutCache.clear();

        if(colorBrushCache.size() > 2000)
            colorBrushCache.clear();

        timeCounter = 0.0f;
    }

    textLayoutTemporary.clear();
}

bool RenderUtil::ScreenChange(std::string& from,std::string &to) {
    std::string currentScreen = GI::getClientInstance()->getScreenName();
    if(currentScreen != from && currentScreen == to) {
        // Screen changed from 'from' to 'to'
        from = currentScreen;  // Update 'from' to the new screen
        return true;           // Indicate that the screen has changed
    } 
    return false;
}


void RenderUtil::Render() {
    Vec2<float> windowsSize = GI::getGuiData()->windowSizeReal;
    ModuleManager::onD2DRender();
}


void RenderUtil::Clean() {
    if(!initD2D)
        return;

    // 先设置标志防止新的渲染调用
    initD2D = false;

    // 等待可能正在进行的渲染操作完成
    if(d2dDeviceContext) {
        d2dDeviceContext->Flush();
        Sleep(16); // 等待一帧时间
    }

    // 释放所有 D2D 资源（按照依赖关系逆序释放）
    SafeRelease(shadowBitmap);   // 新增的阴影位图
    SafeRelease(stencilEffect);  // 新增的模板效果
    SafeRelease(shadowEffect);   // 新增的阴影效果
    SafeRelease(blurEffect);
    SafeRelease(sourceBitmap);
    SafeRelease(d2dDeviceContext);
    SafeRelease(d2dDevice);
    SafeRelease(d2dWriteFactory);
    SafeRelease(d2dFactory);

    // 释放 WIC 工厂
    SafeRelease(wicFactory);

    // 清理 COM 智能指针缓存
    textFormatCache.clear();
    textLayoutCache.clear();
    colorBrushCache.clear();
    textLayoutTemporary.clear();
    urlBitmapCache.clear();
    skinBitmapCache.clear();

    // 清理图片缓存
    imageCacheMap.clear();
    failedUrls.clear();
    failureCount.clear();


    // 确保标志位正确设置
    initD2D = false;
}

void RenderUtil::Flush() {
    d2dDeviceContext->Flush();
}

Vec2<float> RenderUtil::getWindowSize() {
    if(sourceBitmap == nullptr) {
        return Vec2<float>(1920.f, 1080.f);
    }
    D2D1_SIZE_U size = sourceBitmap->GetPixelSize();
    return Vec2<float>((float)size.width, (float)size.height);
}

void RenderUtil::drawText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
                   float textSize, bool storeTextLayout) {
    D2D_CTX_GUARD();
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);

    static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
    if(customFontMod->shadow) {
        ID2D1SolidColorBrush* shadowColorBrush = getSolidColorBrush(UIColor(0, 0, 0, color.a));
        d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x + 1.f, textPos.y + 1.f),
                                         textLayout, shadowColorBrush);
    }

    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawTextLayout(D2D1::Point2F(textPos.x, textPos.y), textLayout, colorBrush);
}

float RenderUtil::getTextWidth(const std::string& textStr, float textSize, bool storeTextLayout) {
    D2D_CTX_GUARD_RET(0.f);
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);
    DWRITE_TEXT_METRICS textMetrics;
    textLayout->GetMetrics(&textMetrics);

    return textMetrics.widthIncludingTrailingWhitespace;
}

float RenderUtil::getTextHeight(const std::string& textStr, float textSize, bool storeTextLayout) {
    D2D_CTX_GUARD_RET(0.f);
    IDWriteTextLayout* textLayout = getTextLayout(textStr, textSize, storeTextLayout);
    DWRITE_TEXT_METRICS textMetrics;
    textLayout->GetMetrics(&textMetrics);

    return std::ceilf(textMetrics.height);
}

void RenderUtil::drawLine(const Vec2<float>& startPos, const Vec2<float>& endPos, const UIColor& color,
                   float width) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawLine(D2D1::Point2F(startPos.x, startPos.y),
                               D2D1::Point2F(endPos.x, endPos.y), colorBrush, width);
}

void RenderUtil::drawRectangle(const Vec4<float>& rect, const UIColor& color, float width) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush, width);
}

void RenderUtil::fillRectangle(const Vec4<float>& rect, const UIColor& color) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->FillRectangle(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), colorBrush);
}

void RenderUtil::drawRoundedRectangle(const Vec4<float>& rect, const UIColor& color, float radius,
                               float width) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    D2D1_ROUNDED_RECT roundedRect =
        D2D1::RoundedRect(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), radius, radius);
    d2dDeviceContext->DrawRoundedRectangle(&roundedRect, colorBrush, width);
}

void RenderUtil::fillRoundedRectangle(const Vec4<float>& rect, const UIColor& color, float radius) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    D2D1_ROUNDED_RECT roundedRect =
        D2D1::RoundedRect(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), radius, radius);
    d2dDeviceContext->FillRoundedRectangle(&roundedRect, colorBrush);
}

void RenderUtil::drawCircle(const Vec2<float>& centerPos, const UIColor& color, float radius,
                     float width) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->DrawEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), colorBrush, width);
}

void RenderUtil::fillCircle(const Vec2<float>& centerPos, const UIColor& color, float radius) {
    D2D_CTX_GUARD();
    ID2D1SolidColorBrush* colorBrush = getSolidColorBrush(color);
    d2dDeviceContext->FillEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerPos.x, centerPos.y), radius, radius), colorBrush);
}

void RenderUtil::addBlur(const Vec4<float>& rect, float strength, bool flush) {
    D2D_CTX_GUARD();
    if(flush) {
        d2dDeviceContext->Flush();
    }
    ID2D1Bitmap* targetBitmap = nullptr;
    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(sourceBitmap->GetPixelFormat());
    d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), props, &targetBitmap);
    D2D1_POINT_2U destPoint = D2D1::Point2U(0, 0);
    D2D1_SIZE_U size = sourceBitmap->GetPixelSize();
    D2D1_RECT_U Rect = D2D1::RectU(0, 0, size.width, size.height);
    targetBitmap->CopyFromBitmap(&destPoint, sourceBitmap, &Rect);

    D2D1_RECT_F screenRectF = D2D1::RectF(0.f, 0.f, (float)sourceBitmap->GetPixelSize().width,
                                          (float)sourceBitmap->GetPixelSize().height);
    D2D1_RECT_F clipRectD2D = D2D1::RectF(rect.x, rect.y, rect.z, rect.w);

    blurEffect->SetInput(0, targetBitmap);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, strength);

    ID2D1Image* outImage = nullptr;
    blurEffect->GetOutput(&outImage);

    ID2D1ImageBrush* outImageBrush = nullptr;
    D2D1_IMAGE_BRUSH_PROPERTIES outImage_props = D2D1::ImageBrushProperties(screenRectF);
    d2dDeviceContext->CreateImageBrush(outImage, outImage_props, &outImageBrush);

    ID2D1RectangleGeometry* clipRectGeo = nullptr;
    d2dFactory->CreateRectangleGeometry(clipRectD2D, &clipRectGeo);
    d2dDeviceContext->FillGeometry(clipRectGeo, outImageBrush);

    targetBitmap->Release();
    outImage->Release();
    outImageBrush->Release();
    clipRectGeo->Release();
}


std::wstring to_wide(const std::string& str) {
    if(str.empty())
        return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if(len <= 0)
        return L"";
    std::wstring wstr(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    return wstr;
}

uint64_t getTextLayoutKey(const std::string& textStr, float textSize) {
    std::hash<std::string> textHash;
    std::hash<float> textSizeHash;
    uint64_t combinedHash = textHash(textStr) ^ textSizeHash(textSize);
    return combinedHash;
}

IDWriteTextFormat* getTextFormat(float textSize) {
    if(textFormatCache[textSize].get() == nullptr) {
        std::wstring fontNameWide = to_wide(currentD2DFont);
        const WCHAR* fontName = fontNameWide.c_str();
        d2dWriteFactory->CreateTextFormat(
            fontName, nullptr, DWRITE_FONT_WEIGHT_NORMAL,
            isFontItalic ? DWRITE_FONT_STYLE_ITALIC
                         : DWRITE_FONT_STYLE_NORMAL,  // DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, (float)currentD2DFontSize * textSize,
            L"zh-cn",  // locale, ʹ�����������������������
            textFormatCache[textSize].put());
    }

    return textFormatCache[textSize].get();
}

IDWriteTextLayout* getTextLayout(const std::string& textStr, float textSize, bool storeTextLayout) {
    std::wstring wideText = to_wide(textStr);
    const WCHAR* text = wideText.c_str();
    IDWriteTextFormat* textFormat = getTextFormat(textSize);
    uint64_t textLayoutKey = getTextLayoutKey(textStr, textSize);

    if(storeTextLayout) {
        if(textLayoutCache[textLayoutKey].get() == nullptr) {
            d2dWriteFactory->CreateTextLayout(text, (UINT32)wcslen(text), textFormat, FLT_MAX, 0.f,
                                              textLayoutCache[textLayoutKey].put());
        }
        return textLayoutCache[textLayoutKey].get();
    } else {
        if(textLayoutTemporary[textLayoutKey].get() == nullptr) {
            d2dWriteFactory->CreateTextLayout(text, (UINT32)wcslen(text), textFormat, FLT_MAX, 0.f,
                                              textLayoutTemporary[textLayoutKey].put());
        }
        return textLayoutTemporary[textLayoutKey].get();
    }
}

ID2D1SolidColorBrush* getSolidColorBrush(const UIColor& color) {
    if(!d2dDeviceContext)
        return nullptr;  // Context not ready �C avoid crash
    uint32_t colorBrushKey = ColorUtil::ColorToUInt(color);
    if(colorBrushCache[colorBrushKey].get() == nullptr) {
        d2dDeviceContext->CreateSolidColorBrush(color.toD2D1Color(),
                                                colorBrushCache[colorBrushKey].put());
    }
    return colorBrushCache[colorBrushKey].get();
}

void RenderUtil::drawTriangle(const Vec4<float>& rect, const UIColor& color, bool leftDirection,
                       bool filled) {
    Vec2<float> p1, p2, p3;
    float midY = (rect.y + rect.w) / 2.f;
    if(leftDirection) {
        p1 = {rect.z, rect.y};
        p2 = {rect.x, midY};
        p3 = {rect.z, rect.w};
    } else {
        p1 = {rect.x, rect.y};
        p2 = {rect.z, midY};
        p3 = {rect.x, rect.w};
    }
    if(filled) {
        ID2D1PathGeometry* geometry = nullptr;
        d2dFactory->CreatePathGeometry(&geometry);
        ID2D1GeometrySink* sink = nullptr;
        geometry->Open(&sink);
        sink->BeginFigure(D2D1::Point2F(p1.x, p1.y), D2D1_FIGURE_BEGIN_FILLED);
        D2D1_POINT_2F points[2] = {D2D1::Point2F(p2.x, p2.y), D2D1::Point2F(p3.x, p3.y)};
        sink->AddLines(points, 2);
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
        sink->Release();
        ID2D1SolidColorBrush* brush = getSolidColorBrush(color);
        d2dDeviceContext->FillGeometry(geometry, brush);
        geometry->Release();
    } else {
        drawLine(p1, p2, color, 2.f);
        drawLine(p2, p3, color, 2.f);
        drawLine(p3, p1, color, 2.f);
    }
}

void RenderUtil::drawPause(const Vec4<float>& rect, const UIColor& color) {
    float barWidth = (rect.z - rect.x) * 0.3f;
    Vec4<float> leftBar{rect.x, rect.y, rect.x + barWidth, rect.w};
    Vec4<float> rightBar{rect.z - barWidth, rect.y, rect.z, rect.w};
    fillRectangle(leftBar, color);
    fillRectangle(rightBar, color);
}

void RenderUtil::fillRect(const Vec2<float>& pos, const Vec2<float>& size, const UIColor& color) {
    Vec4<float> rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
    fillRectangle(rect, color);
}

Vec2<float> RenderUtil::WorldToScreen(const Vec3<float>& worldPos, const Vec2<float>& screenSize) {
    Vec2<float> screenCoords;

    if(Matrix::WorldToScreen(worldPos, screenCoords)) {
        return screenCoords;
    }

    return Vec2<float>(-1, -1);
}

void RenderUtil::drawTextInWorld(const Vec3<float>& worldPos, const std::string& textStr,
                                 const UIColor& color, float textSize) {
    D2D_CTX_GUARD();

    Vec2<float> screenSize = getWindowSize();
    Vec2<float> screenPos = WorldToScreen(worldPos, screenSize);

    if(screenPos.x < 0 || screenPos.y < 0)
        return;

    if(screenPos.x > screenSize.x || screenPos.y > screenSize.y)
        return;

    float textWidth = getTextWidth(textStr, textSize);
    float textHeight = getTextHeight(textStr, textSize);

    screenPos.x -= textWidth * 0.5f;
    screenPos.y -= textHeight * 0.5f;

    drawText(screenPos, textStr, color, textSize);

}

// 添加新的阴影函数
void RenderUtil::addShadow(const Vec4<float>& rect, float strength, const UIColor& shadowColor,
                           float rounding, const std::vector<Vec4<float>>& excludeRects) {
    D2D_CTX_GUARD();

    if(!d2dDeviceContext)
        return;

    Vec2<float> windowSize = getWindowSize();

    // 创建阴影位图（如果还没有）
    if(shadowBitmap == nullptr) {
        D2D1_BITMAP_PROPERTIES1 newLayerProps =
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET, sourceBitmap->GetPixelFormat());
        d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), nullptr, 0, newLayerProps,
                                       &shadowBitmap);
    }

    // 在阴影位图上绘制形状
    d2dDeviceContext->SetTarget(shadowBitmap);
    d2dDeviceContext->Clear(D2D1::ColorF(0, 0, 0, 0));

    // 创建颜色画刷
    ID2D1SolidColorBrush* colorBrush =
        getSolidColorBrush(UIColor(shadowColor.r, shadowColor.g, shadowColor.b, 255));

    // 绘制圆角矩形
    D2D1_ROUNDED_RECT roundedRect =
        D2D1::RoundedRect(D2D1::RectF(rect.x, rect.y, rect.z, rect.w), rounding, rounding);
    d2dDeviceContext->FillRoundedRectangle(roundedRect, colorBrush);

    // 应用高斯模糊
    blurEffect->SetInput(0, shadowBitmap);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, strength);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION,
                         D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY);

    // 如果有排除区域，创建遮罩
    ID2D1Bitmap1* holeMaskBitmap = nullptr;
    if(!excludeRects.empty()) {
        D2D1_BITMAP_PROPERTIES1 maskProps =
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET, sourceBitmap->GetPixelFormat());
        d2dDeviceContext->CreateBitmap(sourceBitmap->GetPixelSize(), nullptr, 0, maskProps,
                                       &holeMaskBitmap);

        d2dDeviceContext->SetTarget(holeMaskBitmap);
        d2dDeviceContext->Clear(D2D1::ColorF(1, 1, 1, 1));  // 白色背景

        // 创建透明画刷来"挖洞"
        ID2D1SolidColorBrush* transparentBrush = nullptr;
        d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f),
                                                &transparentBrush);

        // 保存原始混合模式
        D2D1_PRIMITIVE_BLEND originalBlend = d2dDeviceContext->GetPrimitiveBlend();
        d2dDeviceContext->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);

        // 在排除区域绘制透明矩形
        for(const auto& excludeRect : excludeRects) {
            D2D1_RECT_F rectF =
                D2D1::RectF(excludeRect.x, excludeRect.y, excludeRect.z, excludeRect.w);
            ID2D1RectangleGeometry* rectGeo = nullptr;
            d2dFactory->CreateRectangleGeometry(rectF, &rectGeo);
            d2dDeviceContext->FillGeometry(rectGeo, transparentBrush);
            rectGeo->Release();
        }

        // 恢复混合模式
        d2dDeviceContext->SetPrimitiveBlend(originalBlend);
        transparentBrush->Release();
    }

    // 切换回源位图
    d2dDeviceContext->SetTarget(sourceBitmap);

    // 获取模糊输出
    ID2D1Image* blurOutput = nullptr;
    blurEffect->GetOutput(&blurOutput);

    ID2D1Image* finalOutput = blurOutput;

    // 如果有遮罩，应用 Alpha 遮罩效果
    if(holeMaskBitmap) {
        stencilEffect->SetInput(0, blurOutput);
        stencilEffect->SetInput(1, holeMaskBitmap);
        stencilEffect->GetOutput(&finalOutput);
    }

    // 创建图像画刷
    ID2D1ImageBrush* imageBrush = nullptr;
    D2D1_IMAGE_BRUSH_PROPERTIES brushProps =
        D2D1::ImageBrushProperties(D2D1::RectF(0, 0, windowSize.x, windowSize.y));
    d2dDeviceContext->CreateImageBrush(finalOutput, brushProps, &imageBrush);

    // 设置画刷透明度
    imageBrush->SetOpacity(shadowColor.a / 255.0f);

    // 绘制到整个屏幕
    ID2D1RectangleGeometry* screenGeo = nullptr;
    d2dFactory->CreateRectangleGeometry(D2D1::RectF(0, 0, windowSize.x, windowSize.y), &screenGeo);
    d2dDeviceContext->FillGeometry(screenGeo, imageBrush);

    // 清理资源
    screenGeo->Release();
    imageBrush->Release();
    blurOutput->Release();

    if(holeMaskBitmap) {
        if(finalOutput != blurOutput) {
            finalOutput->Release();
        }
        holeMaskBitmap->Release();
    }
}

void RenderUtil::addDropShadow(const Vec4<float>& rect, float blurRadius,
                               const UIColor& shadowColor, const Vec2<float>& offset,
                               float rounding) {
    D2D_CTX_GUARD();

    // 计算阴影矩形（加上偏移）
    Vec4<float> shadowRect =
        Vec4<float>(rect.x + offset.x, rect.y + offset.y, rect.z + offset.x, rect.w + offset.y);

    // 创建排除原始矩形的列表（避免阴影覆盖原始内容）
    std::vector<Vec4<float>> excludeRects = {rect};

    addShadow(shadowRect, blurRadius, shadowColor, rounding, excludeRects);
}


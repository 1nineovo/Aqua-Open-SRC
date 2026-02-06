#include "DrawUtil.h"

#include "../Libs/glm/glm/glm.hpp"
#include "../Libs/glm/glm/gtc/matrix_transform.hpp"
#include "../Client/Managers/ModuleManager/ModuleManager.h"
#include "../SDK/Render/MeshHelpers.h"
#include <algorithm>
#include <cmath>
#include "../SDK/Render/Matrix.h"
#include "../SDK/Render/BaseActorRenderContext.h"
MinecraftUIRenderContext* DrawUtil::renderCtx = nullptr;
ScreenContext* DrawUtil::screenCtx = nullptr;
ScreenContext* DrawUtil::D3DCTX = nullptr;
ShaderColor* DrawUtil::currentColor = nullptr;
Tessellator* DrawUtil::tessellator = nullptr;
mce::MaterialPtr* DrawUtil::uiMaterial = nullptr;
mce::MaterialPtr* DrawUtil::blendMaterial = nullptr;
LevelRenderer* DrawUtil::levelrenderer = nullptr;
Font* DrawUtil::mcFont = nullptr;
Vec3<float> DrawUtil::origin;
float DrawUtil::deltaTime = 0.016f;
std::vector<DrawUtil::Draw3dData> DrawUtil::quad3dRenderList;
std::vector<DrawUtil::Draw3dData> DrawUtil::line3dRenderList;

void DrawUtil::onRenderScreen(MinecraftUIRenderContext* ctx) {
    renderCtx = ctx;
    screenCtx = ctx->screenContext;
    tessellator = screenCtx->tessellator;
    currentColor = ctx->screenContext->currentShaderColor;
   

	if(uiMaterial == nullptr)
        uiMaterial = mce::MaterialPtr::createMaterial(HashedString("ui_fill_color"));

    if(blendMaterial == nullptr)
        blendMaterial = mce::MaterialPtr::createMaterial(HashedString("fullscreen_cube_overlay_blend"));
    

    mcFont = ctx->clientInstance->minecraftGame->mcFont;

    ModuleManager::onMCRender(renderCtx);
    DrawUtil::renderCtx->flushText(0.f);
}

void DrawUtil::onRenderWorld(LevelRenderer* levelRenderer, ScreenContext* sct) {

    quad3dRenderList.clear();
    line3dRenderList.clear();
    origin = levelRenderer->renderplayer->getorigin();
    ModuleManager::onLevelRender();

  setColor(mce::Color(1.f, 1.f, 1.f, 1.f));
    if(!quad3dRenderList.empty()) {
        tessellator->begin(VertextFormat::QUAD);
        for(auto& quad3dRenderData : quad3dRenderList) {
            mce::Color mcColor = quad3dRenderData.color.toMCColor();
            tessellator->color(mcColor.r, mcColor.g, mcColor.b, mcColor.a);
            tessellator->vertex(quad3dRenderData.pos.x, quad3dRenderData.pos.y,
                                quad3dRenderData.pos.z);
        }
        MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
        quad3dRenderList.clear();
    }

    if(!line3dRenderList.empty()) {
        tessellator->begin(VertextFormat::LINE_LIST);
        for(auto& line3dRenderData : line3dRenderList) {
            mce::Color mcColor = line3dRenderData.color.toMCColor();
            tessellator->color(mcColor.r, mcColor.g, mcColor.b, mcColor.a);
            tessellator->vertex(line3dRenderData.pos.x, line3dRenderData.pos.y,
                                line3dRenderData.pos.z);
        }
        MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
        line3dRenderList.clear();
    }
}
    
void DrawUtil::setColor(const mce::Color& color) {
    currentColor->color = color;
    currentColor->dirty = true;
}

void DrawUtil::setColor(const UIColor& color) {
    currentColor->color = color.toMCColor();
    currentColor->dirty = true;
}

 void DrawUtil::drawText(const Vec2<float>& textPos, const std::string& textStr, const UIColor& color,
                   float textSize) {
    std::string text = textStr;
    mce::Color mcColor = color.toMCColor();

    RectangleArea rect = {};
    rect._x0 = textPos.x;
    rect._x1 = textPos.x;
    rect._y0 = textPos.y;
    rect._y1 = textPos.y;

    TextMeasureData textMeasureData = {};
    textMeasureData.fontSize = textSize;
    textMeasureData.linePadding = 0.f;
    textMeasureData.renderShadow = true;
    textMeasureData.showColorSymbol = false;
    textMeasureData.hideHyphen = false;

    static CaretMeasureData caretMeasureData = {};
    caretMeasureData.position = -1.f;
    caretMeasureData.shouldRender = false;

    renderCtx->drawText(mcFont, &rect, &text, &mcColor, mcColor.a, 0, &textMeasureData,
                        &caretMeasureData);
}


void DrawUtil::drawTextInWorld(const Vec3<float>& pos, const std::string& textStr,
                               const UIColor& color, float textSize) {
    Vec2<float> textPos;
    const float textWidth = getTextWidth(textStr, textSize);
    const float textHeight = getTextHeight(textSize);

    if(Matrix::WorldToScreen(pos.add(0.5f), textPos)) {
        textPos.y -= textHeight;
        textPos.x -= textWidth / 2;

        drawText(textPos, textStr, color, textSize);

        fillRectangle(Vec4<float>(textPos.x - 5, textPos.y - 5, textPos.x + 50, textPos.y + 20),
                      UIColor(255, 0, 0, 100)); 
    }
}
 float DrawUtil::getTextWidth(const std::string& textStr, float textSize) {
     if(renderCtx == nullptr || mcFont == nullptr) {
         return static_cast<float>(textStr.size()) * 6.f * textSize;
     }

     std::string text = textStr;
     if(!renderCtx || !mcFont) {
         return 0.0f;
     }
     return mcFont->getLineLength(&text, textSize, false);
 }

 float DrawUtil::getTextHeight(float textSize) {
     if(mcFont == nullptr)
         return 9.f * textSize;

     return 9.f * textSize;
 }
 void DrawUtil::drawQuad(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3,
                    const Vec2<float>& p4, const UIColor& color) {
     setColor(color);
     tessellator->begin(VertextFormat::QUAD, 4);

     tessellator->vertex(p1.x, p1.y, 0.f);
     tessellator->vertex(p2.x, p2.y, 0.f);
     tessellator->vertex(p3.x, p3.y, 0.f);
     tessellator->vertex(p4.x, p4.y, 0.f);

     MeshHelpers::renderMeshImmediately(screenCtx, tessellator, uiMaterial);
 }

 void DrawUtil::fillRectangle(const Vec4<float>& rect, const UIColor& color) {
     drawQuad(Vec2<float>(rect.x, rect.w), Vec2<float>(rect.z, rect.w), Vec2<float>(rect.z, rect.y),
              Vec2<float>(rect.x, rect.y), color);
 }
 void DrawUtil::drawImage(mce::TexturePtr& texture, Vec2<float> const& pos, Vec2<float> const& size,
                            mce::Color const& flushCol) {
     renderCtx->drawImage(&texture, pos, size, {0.f, 0.f},
                          {1.f, 1.f});
     renderCtx->flushImages(flushCol, 1.f, HashedString("ui_grayscale") /*random hashed string*/);
 }

void DrawUtil::drawItem(const Vec2<float>& pos, ItemStack* itemStack, float scale,
                   float opacity) {
    if (!itemStack || !renderCtx || !renderCtx->screenContext || !renderCtx->clientInstance) {
        return;
    }
    
    if (itemStack == ItemStack::getEmptyItem() || !itemStack->getItem()) {
        return;
    }

    auto baseCtx = std::make_unique<BaseActorRenderContext>(
        renderCtx->screenContext, 
        renderCtx->clientInstance, 
        renderCtx->clientInstance->minecraftGame
    );
    if (!baseCtx || !baseCtx->itemRenderer) {
        return;
    }

    auto it = itemStack->mItem.get();
    if (!it) {
        return;
    }
    baseCtx->itemRenderer->renderGuiItemNew(baseCtx.get(), itemStack, 0, pos.x, pos.y, opacity, scale, 0.f, false);
    if (it->isGlint(itemStack)) {
        baseCtx->itemRenderer->renderGuiItemNew(baseCtx.get(), itemStack, 0, pos.x, pos.y, opacity, scale, 0.f, true);
    }
}


 void DrawUtil::drawBox3dFilled(const AABB& aabb, const UIColor& color, const UIColor& lineColor,
                           float scale) {
     Vec3<float> diff = aabb.upper.sub(aabb.lower);
     Vec3<float> newLower = aabb.lower.sub(DrawUtil::origin);

     Vec3<float> vertices[8] = {{newLower.x, newLower.y, newLower.z},
                                {newLower.x + diff.x, newLower.y, newLower.z},
                                {newLower.x, newLower.y, newLower.z + diff.z},
                                {newLower.x + diff.x, newLower.y, newLower.z + diff.z},

                                {newLower.x, newLower.y + diff.y, newLower.z},
                                {newLower.x + diff.x, newLower.y + diff.y, newLower.z},
                                {newLower.x, newLower.y + diff.y, newLower.z + diff.z},
                                {newLower.x + diff.x, newLower.y + diff.y, newLower.z + diff.z}};

     glm::mat4 rotationMatrix = glm::rotate(glm::mat4(scale), 0.f, glm::vec3(1.0f, 1.0f, 1.0f));
     Vec3<float> newLowerReal = newLower.add(Vec3<float>(0.5f, 0.5f, 0.5f));

     for(int i = 0; i < 8; i++) {
         glm::vec4 rotatedVertex = rotationMatrix * glm::vec4(vertices[i].x - newLowerReal.x,
                                                              vertices[i].y - newLowerReal.y,
                                                              vertices[i].z - newLowerReal.z, 0.0f);
         vertices[i] =
             Vec3<float>(rotatedVertex.x + newLowerReal.x, rotatedVertex.y + newLowerReal.y,
                         rotatedVertex.z + newLowerReal.z);
     }

     if(color.a > 0) {
         static uint8_t v[48] = {5, 7, 6, 4, 4, 6, 7, 5, 1, 3, 2, 0, 0, 2, 3, 1,
                                 4, 5, 1, 0, 0, 1, 5, 4, 6, 7, 3, 2, 2, 3, 7, 6,
                                 4, 6, 2, 0, 0, 2, 6, 4, 5, 7, 3, 1, 1, 3, 7, 5};
         for(int i = 0; i < 48; i++)
             quad3dRenderList.push_back(Draw3dData(vertices[v[i]], color));
     }

     if(lineColor.a > 0) {
         static auto drawLine = [](const Vec3<float>& start, const Vec3<float>& end,
                                   const UIColor& _color) {
             line3dRenderList.push_back(Draw3dData(start, _color));
             line3dRenderList.push_back(Draw3dData(end, _color));
         };

         // Top square
         drawLine(vertices[4], vertices[5], lineColor);
         drawLine(vertices[5], vertices[7], lineColor);
         drawLine(vertices[7], vertices[6], lineColor);
         drawLine(vertices[6], vertices[4], lineColor);

         // Bottom Square
         drawLine(vertices[0], vertices[1], lineColor);
         drawLine(vertices[1], vertices[3], lineColor);
         drawLine(vertices[3], vertices[2], lineColor);
         drawLine(vertices[2], vertices[0], lineColor);

         // Sides
         drawLine(vertices[0], vertices[4], lineColor);
         drawLine(vertices[1], vertices[5], lineColor);
         drawLine(vertices[2], vertices[6], lineColor);
         drawLine(vertices[3], vertices[7], lineColor);
     }
 }

 void DrawUtil::drawBoxFilledPre(const Vec3<float>& p1, const Vec3<float>& p2, const UIColor& color,
                            const UIColor& lineColor) {
     AABB aabb;
     aabb.lower =
         Vec3<float>((std::min)(p1.x, p2.x), (std::min)(p1.y, p2.y), (std::min)(p1.z, p2.z));
     aabb.upper =
         Vec3<float>((std::max)(p1.x, p2.x), (std::max)(p1.y, p2.y), (std::max)(p1.z, p2.z));
     drawBox3dFilled(aabb, color, lineColor, 1.f);
 }

 void DrawUtil::drawBoxFilled(const Vec3<float>& blockPos, const UIColor& color,
                           const UIColor& lineColor) {
     Vec3<float> lower(blockPos.x - 0.5f, blockPos.y-0.5f, blockPos.z-0.5f);
     Vec3<float> upper = lower.add(Vec3<float>(1.f, 1.f, 1.f));
     drawBoxFilledPre(lower, upper, color, lineColor);
 }

void DrawUtil::drawBox3dFilledRotation(const AABB& aabb, const UIColor& color, const UIColor& lineColor,
                                   float scale, BoxRotation rotation) {
     Vec3<float> diff = aabb.upper.sub(aabb.lower);
    Vec3<float> newLower = aabb.lower.sub(DrawUtil::origin);

     Vec3<float> vertices[8] = {{newLower.x, newLower.y, newLower.z},
                                {newLower.x + diff.x, newLower.y, newLower.z},
                                {newLower.x, newLower.y, newLower.z + diff.z},
                                {newLower.x + diff.x, newLower.y, newLower.z + diff.z},

                                {newLower.x, newLower.y + diff.y, newLower.z},
                                {newLower.x + diff.x, newLower.y + diff.y, newLower.z},
                                {newLower.x, newLower.y + diff.y, newLower.z + diff.z},
                                {newLower.x + diff.x, newLower.y + diff.y, newLower.z + diff.z}};

     Vec3<float> center = newLower.add(diff.div(Vec3<float>(2.f, 2.f, 2.f)));

     glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));

     glm::vec3 axis(rotation.rotX, rotation.rotY, rotation.rotZ);
     float angle = glm::length(axis);
     if(angle > 0.00001f) {
         axis = glm::normalize(axis);
         transform = glm::rotate(transform, angle, axis);
     }

     for(int i = 0; i < 8; i++) {
         glm::vec4 offset(vertices[i].x - center.x, vertices[i].y - center.y,
                          vertices[i].z - center.z, 1.0f);
         glm::vec4 res = transform * offset;
         vertices[i] = Vec3<float>(res.x + center.x, res.y + center.y, res.z + center.z);
     }

     if(color.a > 0) {
         static uint8_t v[48] = {5, 7, 6, 4, 4, 6, 7, 5, 1, 3, 2, 0, 0, 2, 3, 1,
                                 4, 5, 1, 0, 0, 1, 5, 4, 6, 7, 3, 2, 2, 3, 7, 6,
                                 4, 6, 2, 0, 0, 2, 6, 4, 5, 7, 3, 1, 1, 3, 7, 5};
         for(int i = 0; i < 48; i++)
             quad3dRenderList.push_back(Draw3dData(vertices[v[i]], color));
     }

     if(lineColor.a > 0) {
         static auto drawLine = [](const Vec3<float>& start, const Vec3<float>& end,
                                   const UIColor& _color) {
             line3dRenderList.push_back(Draw3dData(start, _color));
             line3dRenderList.push_back(Draw3dData(end, _color));
         };

         drawLine(vertices[4], vertices[5], lineColor);
         drawLine(vertices[5], vertices[7], lineColor);
         drawLine(vertices[7], vertices[6], lineColor);
         drawLine(vertices[6], vertices[4], lineColor);

         drawLine(vertices[0], vertices[1], lineColor);
         drawLine(vertices[1], vertices[3], lineColor);
         drawLine(vertices[3], vertices[2], lineColor);
         drawLine(vertices[2], vertices[0], lineColor);

         drawLine(vertices[0], vertices[4], lineColor);
         drawLine(vertices[1], vertices[5], lineColor);
         drawLine(vertices[2], vertices[6], lineColor);
         drawLine(vertices[3], vertices[7], lineColor);
     }



 }

void DrawUtil::drawBox3dFilledRotP(const Vec3<float>& center, const Vec3<float>& halfSize,
                               float pitchRad, float yawRad, const UIColor& color,
                               const UIColor& lineColor) {
     float cp = cosf(pitchRad), sp = sinf(pitchRad);
     float cy = cosf(yawRad), sy = sinf(yawRad);

     auto rotXPY = [&](const Vec3<float>& v) {
         // X-axis rotation
         float x1 = v.x;
         float y1 = v.y * cp - v.z * sp;
         float z1 = v.y * sp + v.z * cp;

         // Y-axis rotation
         float x2 = x1 * cy - z1 * sy;
         float z2 = x1 * sy + z1 * cy;
         return Vec3<float>(x2, y1, z2);
     };

     // ���ɾֲ� 8 ���㲢���任
     Vec3<float> world[8];
     int idx = 0;
     for(int dx = -1; dx <= 1; dx += 2)
         for(int dy = -1; dy <= 1; dy += 2)
             for(int dz = -1; dz <= 1; dz += 2)
                 world[idx++] =
                     rotXPY(Vec3<float>(halfSize.x * dx, halfSize.y * dy, halfSize.z * dz))
                         .add(center)
                         .sub(origin);

     // --- ��� ---
     if(color.a) {
         setColor(color);
         tessellator->begin(VertextFormat::TRIANGLE_LIST);
         mce::Color mc = color.toMCColor();
         auto quad = [&](int a, int b, int c, int d) {
             tessellator->color(mc.r, mc.g, mc.b, mc.a);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[b].x, world[b].y, world[b].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[d].x, world[d].y, world[d].z);
         };
         quad(0, 1, 3, 2);
         quad(4, 6, 7, 5);  // bottom & top
         quad(0, 2, 6, 4);
         quad(1, 5, 7, 3);  // -X & +X
         quad(0, 4, 5, 1);
         quad(2, 3, 7, 6);  // -Z & +Z
         MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
     }

     // --- ��� ---
     if(lineColor.a) {
         auto line = [&](int a, int b) {
             line3dRenderList.emplace_back(world[a], lineColor);
             line3dRenderList.emplace_back(world[b], lineColor);
         };
         line(0, 1);
         line(1, 3);
         line(3, 2);
         line(2, 0);
         line(4, 5);
         line(5, 7);
         line(7, 6);
         line(6, 4);
         line(0, 4);
         line(1, 5);
         line(2, 6);
         line(3, 7);
     }
 }

 void DrawUtil::drawBox3dFilledRotP_BottomCenter(const Vec3<float>& center, const Vec3<float>& halfSize,
                                            float pitchRad, float yawRad, const UIColor& color,
                                            const UIColor& lineColor) {
     float cp = cosf(pitchRad), sp = sinf(pitchRad);
     float cy = cosf(yawRad), sy = sinf(yawRad);

     Vec3<float> right(cy, 0.f, -sy);

     auto rotYawPitchBottom = [&](const Vec3<float>& v) {
         float x1 = v.x * cy - v.z * sy;
         float z1 = v.x * sy + v.z * cy;
         float y1 = v.y;

         y1 -= halfSize.y;  // ������ -halfSize.y���Ƶ� 0

         Vec3<float> k = right;
         Vec3<float> cross(k.y * z1 - k.z * y1, k.z * x1 - k.x * z1, k.x * y1 - k.y * x1);
         float dot = k.x * x1 + k.y * y1 + k.z * z1;

         Vec3<float> r;
         r.x = x1 * cp + cross.x * sp + k.x * dot * (1 - cp);
         r.y = y1 * cp + cross.y * sp + k.y * dot * (1 - cp);
         r.z = z1 * cp + cross.z * sp + k.z * dot * (1 - cp);

         r.y -= halfSize.y;
         return r;
     };

     Vec3<float> world[8];
     int idx = 0;
     for(int dx = -1; dx <= 1; dx += 2)
         for(int dy = -1; dy <= 1; dy += 2)
             for(int dz = -1; dz <= 1; dz += 2)
                 world[idx++] = rotYawPitchBottom(
                                    Vec3<float>(halfSize.x * dx, halfSize.y * dy, halfSize.z * dz))
                                    .add(center)
                                    .sub(origin);

     if(color.a) {
         setColor(color);
         tessellator->begin(VertextFormat::TRIANGLE_LIST);
         mce::Color mc = color.toMCColor();
         auto quad = [&](int a, int b, int c, int d) {
             tessellator->color(mc.r, mc.g, mc.b, mc.a);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[b].x, world[b].y, world[b].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[d].x, world[d].y, world[d].z);
         };
         quad(0, 1, 3, 2);
         quad(4, 6, 7, 5);
         quad(0, 2, 6, 4);
         quad(1, 5, 7, 3);
         quad(0, 4, 5, 1);
         quad(2, 3, 7, 6);
         MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
     }

     if(lineColor.a) {
         auto line = [&](int a, int b) {
             line3dRenderList.emplace_back(world[a], lineColor);
             line3dRenderList.emplace_back(world[b], lineColor);
         };
         line(0, 1);
         line(1, 3);
         line(3, 2);
         line(2, 0);
         line(4, 5);
         line(5, 7);
         line(7, 6);
         line(6, 4);
         line(0, 4);
         line(1, 5);
         line(2, 6);
         line(3, 7);
     }
 }

 void DrawUtil::drawBox3dFilledRotP_TopCenter(const Vec3<float>& center, const Vec3<float>& halfSize,
                                         float pitchRad, float yawRad, const UIColor& color,
                                         const UIColor& lineColor) {
     float cp = cosf(pitchRad), sp = sinf(pitchRad);
     float cy = cosf(yawRad), sy = sinf(yawRad);

     Vec3<float> right(cy, 0.f, -sy);

     auto rotYawPitchBottom = [&](const Vec3<float>& v) {
         float x1 = v.x * cy - v.z * sy;
         float z1 = v.x * sy + v.z * cy;
         float y1 = v.y;

         y1 += halfSize.y;  // ������ -halfSize.y���Ƶ� 0

         Vec3<float> k = right;
         Vec3<float> cross(k.y * z1 - k.z * y1, k.z * x1 - k.x * z1, k.x * y1 - k.y * x1);
         float dot = k.x * x1 + k.y * y1 + k.z * z1;

         Vec3<float> r;
         r.x = x1 * cp + cross.x * sp + k.x * dot * (1 - cp);
         r.y = y1 * cp + cross.y * sp + k.y * dot * (1 - cp);
         r.z = z1 * cp + cross.z * sp + k.z * dot * (1 - cp);

         r.y -= halfSize.y;
         return r;
     };

     Vec3<float> world[8];
     int idx = 0;
     for(int dx = -1; dx <= 1; dx += 2)
         for(int dy = -1; dy <= 1; dy += 2)
             for(int dz = -1; dz <= 1; dz += 2)
                 world[idx++] = rotYawPitchBottom(
                                    Vec3<float>(halfSize.x * dx, halfSize.y * dy, halfSize.z * dz))
                                    .add(center)
                                    .sub(origin);

     if(color.a) {
         setColor(color);
         tessellator->begin(VertextFormat::TRIANGLE_LIST);
         mce::Color mc = color.toMCColor();
         auto quad = [&](int a, int b, int c, int d) {
             tessellator->color(mc.r, mc.g, mc.b, mc.a);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[b].x, world[b].y, world[b].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[a].x, world[a].y, world[a].z);
             tessellator->vertex(world[c].x, world[c].y, world[c].z);
             tessellator->vertex(world[d].x, world[d].y, world[d].z);
         };
         quad(0, 1, 3, 2);
         quad(4, 6, 7, 5);
         quad(0, 2, 6, 4);
         quad(1, 5, 7, 3);
         quad(0, 4, 5, 1);
         quad(2, 3, 7, 6);
         MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
     }

     if(lineColor.a) {
         auto line = [&](int a, int b) {
             line3dRenderList.emplace_back(world[a], lineColor);
             line3dRenderList.emplace_back(world[b], lineColor);
         };
         line(0, 1);
         line(1, 3);
         line(3, 2);
         line(2, 0);
         line(4, 5);
         line(5, 7);
         line(7, 6);
         line(6, 4);
         line(0, 4);
         line(1, 5);
         line(2, 6);
         line(3, 7);
     }
 }


 void DrawUtil::drawLine3d(const Vec3<float>& start, const Vec3<float>& end, const UIColor& color) {
     line3dRenderList.push_back(Draw3dData(start.sub(origin), color));
     line3dRenderList.push_back(Draw3dData(end.sub(origin), color));
 }

 void DrawUtil::drawCircle3d(const Vec3<float>& center, float radius, const UIColor& color,
                        int segments) {
     if(segments < 3)
         segments = 3;
     float step = 2.f * PI / segments;
     Vec3<float> prev(center.x + radius, center.y, center.z);
     for(int i = 1; i <= segments; ++i) {
         float ang = step * i;
         Vec3<float> curr(center.x + radius * cosf(ang), center.y, center.z + radius * sinf(ang));
         drawLine3d(prev, curr, color);
         prev = curr;
     }
 }

 void DrawUtil::drawCylinder3d(const Vec3<float>& baseCenter, float radius, float height,
                          const UIColor& color, int segments) {
     if(segments < 3)
         segments = 3;
     // Bottom and top circles
     drawCircle3d(baseCenter, radius, color, segments);
     Vec3<float> topCenter = baseCenter.add(Vec3<float>(0.f, height, 0.f));
     drawCircle3d(topCenter, radius, color, segments);
     // Vertical lines
     float step = 2.f * PI / segments;
     for(int i = 0; i < segments; ++i) {
         float ang = step * i;
         Vec3<float> bottom(baseCenter.x + radius * cosf(ang), baseCenter.y,
                            baseCenter.z + radius * sinf(ang));
         Vec3<float> top = bottom.add(Vec3<float>(0.f, height, 0.f));
         drawLine3d(bottom, top, color);
     }
 }

 void DrawUtil::drawSphere3d(const Vec3<float>& center, float radius, const UIColor& color,
                        int segments) {
     if(segments < 6)
         segments = 6;
     // three great circles: XZ, XY, YZ planes
     drawCircle3d(center, radius, color, segments);  // XZ plane
     // XY plane
     float step = 2.f * PI / segments;
     Vec3<float> prev(center.x + radius, center.y, center.z);
     for(int i = 1; i <= segments; ++i) {
         float ang = step * i;
         Vec3<float> curr(center.x + radius * cosf(ang), center.y + radius * sinf(ang), center.z);
         drawLine3d(prev, curr, color);
         prev = curr;
     }
     // YZ plane
     prev = Vec3<float>(center.x, center.y + radius, center.z);
     for(int i = 1; i <= segments; ++i) {
         float ang = step * i;
         Vec3<float> curr(center.x, center.y + radius * cosf(ang), center.z + radius * sinf(ang));
         drawLine3d(prev, curr, color);
         prev = curr;
     }
 }

 void DrawUtil::drawSphere3dFilled(const Vec3<float>& center, float radius, const UIColor& color,
                              int stacks, int slices) {
     if(stacks < 2)
         stacks = 2;
     if(slices < 3)
         slices = 3;

     // Prepare tessellator
     setColor(color);
     tessellator->begin(VertextFormat::TRIANGLE_LIST);

     mce::Color mc = color.toMCColor();

     for(int lat = 0; lat < stacks; ++lat) {
         float phi1 = PI * ((float)lat / stacks) - PI / 2.0f;  // -90 to 90
         float phi2 = PI * ((float)(lat + 1) / stacks) - PI / 2.0f;

         float sinPhi1 = sinf(phi1);
         float cosPhi1 = cosf(phi1);
         float sinPhi2 = sinf(phi2);
         float cosPhi2 = cosf(phi2);

         for(int lon = 0; lon < slices; ++lon) {
             float theta1 = 2.0f * PI * (float)lon / slices;
             float theta2 = 2.0f * PI * (float)(lon + 1) / slices;

             float sinTheta1 = sinf(theta1);
             float cosTheta1 = cosf(theta1);
             float sinTheta2 = sinf(theta2);
             float cosTheta2 = cosf(theta2);

             // four vertices of quad on sphere surface
             Vec3<float> v1(center.x + radius * cosPhi1 * cosTheta1, center.y + radius * sinPhi1,
                            center.z + radius * cosPhi1 * sinTheta1);
             Vec3<float> v2(center.x + radius * cosPhi2 * cosTheta1, center.y + radius * sinPhi2,
                            center.z + radius * cosPhi2 * sinTheta1);
             Vec3<float> v3(center.x + radius * cosPhi2 * cosTheta2, center.y + radius * sinPhi2,
                            center.z + radius * cosPhi2 * sinTheta2);
             Vec3<float> v4(center.x + radius * cosPhi1 * cosTheta2, center.y + radius * sinPhi1,
                            center.z + radius * cosPhi1 * sinTheta2);

             // subtract camera origin
             v1 = v1.sub(origin);
             v2 = v2.sub(origin);
             v3 = v3.sub(origin);
             v4 = v4.sub(origin);

             tessellator->color(mc.r, mc.g, mc.b, mc.a);
             tessellator->vertex(v1.x, v1.y, v1.z);
             tessellator->vertex(v2.x, v2.y, v2.z);
             tessellator->vertex(v3.x, v3.y, v3.z);

             tessellator->vertex(v1.x, v1.y, v1.z);
             tessellator->vertex(v3.x, v3.y, v3.z);
             tessellator->vertex(v4.x, v4.y, v4.z);
         }
     }

     MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
 }

 void DrawUtil::drawCylinder3dFilled(const Vec3<float>& baseCenter, float radius, float height,
                                const UIColor& color, int slices) {
     if(slices < 3)
         slices = 3;
     setColor(color);
     tessellator->begin(VertextFormat::TRIANGLE_LIST);
     mce::Color mc = color.toMCColor();

     float yBottom = baseCenter.y;
     float yTop = baseCenter.y + height;

     for(int i = 0; i < slices; ++i) {
         float theta1 = 2.f * PI * (float)i / slices;
         float theta2 = 2.f * PI * (float)(i + 1) / slices;

         // Rim points
         Vec3<float> p1(baseCenter.x + radius * cosf(theta1), yBottom,
                        baseCenter.z + radius * sinf(theta1));
         Vec3<float> p2(baseCenter.x + radius * cosf(theta2), yBottom,
                        baseCenter.z + radius * sinf(theta2));
         Vec3<float> p3(baseCenter.x + radius * cosf(theta1), yTop,
                        baseCenter.z + radius * sinf(theta1));
         Vec3<float> p4(baseCenter.x + radius * cosf(theta2), yTop,
                        baseCenter.z + radius * sinf(theta2));

         // subtract origin
         p1 = p1.sub(origin);
         p2 = p2.sub(origin);
         p3 = p3.sub(origin);
         p4 = p4.sub(origin);

         // Side quad (two triangles)
         tessellator->color(mc.r, mc.g, mc.b, mc.a);
         tessellator->vertex(p1.x, p1.y, p1.z);
         tessellator->vertex(p3.x, p3.y, p3.z);
         tessellator->vertex(p4.x, p4.y, p4.z);

         tessellator->vertex(p1.x, p1.y, p1.z);
         tessellator->vertex(p4.x, p4.y, p4.z);
         tessellator->vertex(p2.x, p2.y, p2.z);

         // Top cap
         Vec3<float> topCenter = Vec3<float>(baseCenter.x, yTop, baseCenter.z).sub(origin);
         tessellator->vertex(topCenter.x, topCenter.y, topCenter.z);
         tessellator->vertex(p4.x, p4.y, p4.z);
         tessellator->vertex(p3.x, p3.y, p3.z);

         // Bottom cap
         Vec3<float> bottomCenter = Vec3<float>(baseCenter.x, yBottom, baseCenter.z).sub(origin);
         tessellator->vertex(bottomCenter.x, bottomCenter.y, bottomCenter.z);
         tessellator->vertex(p2.x, p2.y, p2.z);
         tessellator->vertex(p1.x, p1.y, p1.z);
     }

     MeshHelpers::renderMeshImmediately(screenCtx, tessellator, blendMaterial);
 }


 void DrawUtil::drawPlayerModel(const Vec3<float>& position, float pitch, float yaw,
                                      const UIColor& color, const UIColor& lineColor) {

  
     float yawRad = yaw * PI / 180.f;
     float pitchRad = pitch * PI / 180.f;


     // 模型尺寸定义（来自 PlayerCham.h）
     static const Vec3<float> headHalf = Vec3<float>(0.25f, 0.25f, 0.25f);
     static const Vec3<float> bodyHalf = Vec3<float>(0.3f, 0.35f, 0.15f);
     static const Vec3<float> armHalf = Vec3<float>(0.15f, 0.35f, 0.15f);
     static const Vec3<float> legHalf = Vec3<float>(0.15f, 0.35f, 0.15f);

     // 肢体相对位置偏移
     static const Vec3<float> localLeftArmOffset = Vec3<float>(-0.4f, 0.7f, 0.0f);
     static const Vec3<float> localRightArmOffset = Vec3<float>(0.4f, 0.7f, 0.0f);
     static const Vec3<float> localLeftLegOffset = Vec3<float>(-0.15f, 0.f, 0.0f);
     static const Vec3<float> localRightLegOffset = Vec3<float>(0.15f, 0.f, 0.0f);

     // 计算旋转值
     float cy = cosf(yawRad), sy = sinf(yawRad);

     // 世界坐标变换函数
     auto toWorld = [&](const Vec3<float>& local) {
         return Vec3<float>(local.x * cy - local.z * sy, local.y, local.x * sy + local.z * cy);
     };

     // 计算各部位中心点
     Vec3<float> headCenter = position;
     Vec3<float> bodyCenter = headCenter.add(Vec3<float>(0.f, -0.55f, 0.f));
     Vec3<float> leftArmCenter = bodyCenter.add(toWorld(localLeftArmOffset));
     Vec3<float> rightArmCenter = bodyCenter.add(toWorld(localRightArmOffset));
     Vec3<float> leftLegCenter = bodyCenter.add(toWorld(localLeftLegOffset));
     Vec3<float> rightLegCenter = bodyCenter.add(toWorld(localRightLegOffset));

     // 绘制头部
     drawBox3dFilledRotP(headCenter, headHalf, pitchRad, yawRad, color, lineColor);

     // 绘制身体（使用 TopCenter 锚点）
     drawBox3dFilledRotP_TopCenter(bodyCenter, bodyHalf, 0.f, yawRad, color, lineColor);

     // 绘制手臂（静态姿势，稍微向下）
     float staticArmPitch = 0.2f;  // 大约11.5度向下
     drawBox3dFilledRotP_BottomCenter(leftArmCenter, armHalf, staticArmPitch, yawRad, color,
                                      lineColor);
     drawBox3dFilledRotP_BottomCenter(rightArmCenter, armHalf, staticArmPitch, yawRad, color,
                                      lineColor);

     // 绘制腿部（静态姿势，垂直向下）
     drawBox3dFilledRotP_BottomCenter(leftLegCenter, legHalf, 0.f, yawRad, color, lineColor);
     drawBox3dFilledRotP_BottomCenter(rightLegCenter, legHalf, 0.f, yawRad, color, lineColor);
 }

 void DrawUtil::drawLine(const Vec3<float>& start, const Vec3<float>& end, const UIColor& color) {
     Vec3<float> renderStart = start.sub(DrawUtil::origin);
     Vec3<float> renderEnd = end.sub(DrawUtil::origin);

     line3dRenderList.push_back(Draw3dData(renderStart, color));
     line3dRenderList.push_back(Draw3dData(renderEnd, color));
 }


 void DrawUtil::drawRing3d(const Vec3<float>& pos, float radius, const UIColor& color,
                           float lineWidth) {
     constexpr int segments = 30;
     constexpr float step = (3.1415926535f * 2.0f) / segments;

     for(int i = 0; i < segments; i++) {
         float theta = i * step;
         float thetaNext = (i + 1) * step;
         Vec3<float> p1(pos.x + radius * cos(theta), pos.y, pos.z + radius * sin(theta));

         Vec3<float> p2(pos.x + radius * cos(thetaNext), pos.y, pos.z + radius * sin(thetaNext));

         drawLine(p1, p2, color);
     }
 }
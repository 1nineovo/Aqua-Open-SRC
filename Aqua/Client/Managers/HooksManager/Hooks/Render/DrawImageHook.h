#pragma once
#include "../../Client/Client.h"
#include "../FuncHook.h"
#include <DrawUtil.h>
Vec2<float> hotbarPos = Vec2<float>(0, 0);


class DrawImageHook : public FuncHook {
   private:
    using func_t = __int64(__fastcall*)(MinecraftUIRenderContext*, mce::TexturePtr*, Vec2<float>*,
                                        Vec2<float>*, Vec2<float>*, Vec2<float>*, void*);
    static inline func_t oFunc;

    static __int64 DrawImageCallback(MinecraftUIRenderContext* ctx, mce::TexturePtr* texture,
                                     Vec2<float>* pos, Vec2<float>* size, Vec2<float>* uvPos,
                                     Vec2<float>* uvSize, void* a7) {
        if(!texture || !texture->mResourceLocation)
            return oFunc(ctx, texture, pos, size, uvPos, uvSize, a7);
        if(!GI::getClientInstance())
            return oFunc(ctx, texture, pos, size, uvPos, uvSize, a7);

        static Interface* interfaceMod = ModuleManager::getModule<Interface>();

        std::string screenName = GI::getClientInstance()->getScreenName();

        float scale = GI::getGuiData()->Scale;
        if(screenName == "hud_screen") {
            if(uvPos->x == 0 && uvPos->y == 0 && uvSize->x == 1 && uvSize->y == 1 &&
               size->x == 24 && size->y == 24) {  // Selected Hotbar Image
                static float lerpedPos = pos->x;
                lerpedPos = Math::animate(pos->x, lerpedPos, 0.5f);
                pos->x = lerpedPos;
                
                auto rectPos =
                    Vec4<float>(pos->x + 2, pos->y + 2, pos->x + size->x - 2, pos->y + size->y - 2);
                auto barPos =
                    Vec4<float>(pos->x + 1, pos->y + 1, pos->x + size->x - 1, pos->y + 1.5f);

                
                auto realRectPos =
                    Vec4<float>(rectPos.x * scale, rectPos.y * scale, rectPos.z * scale, rectPos.w * scale);

                if(interfaceMod->isEnabled() && interfaceMod->hotbar) {
                    interfaceMod->sHotbarRectPos = realRectPos;
                    return 0;
                }
            }
        }

        return oFunc(ctx, texture, pos, size, uvPos, uvSize, a7);
    }

   public:
    DrawImageHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&DrawImageCallback;
    }
};

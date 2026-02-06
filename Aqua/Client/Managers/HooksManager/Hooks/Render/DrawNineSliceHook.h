#pragma once
#include <DrawUtil.h>

#include "../../Client/Client.h"
#include "../FuncHook.h"

class DrawNiceSliceHook : public FuncHook {
   private:
    using func_t = __int64(__fastcall*)(MinecraftUIRenderContext*, mce::TexturePtr*, NinesliceInfo* );
    static inline func_t oFunc;

    static __int64 DrawNiceSliceCallback(MinecraftUIRenderContext* ctx, mce::TexturePtr* texture,
                                         NinesliceInfo* info) {
        if(!texture || !texture->mResourceLocation)
            return oFunc(ctx, texture, info);

        return oFunc(ctx, texture, info);
    }

   public:
    DrawNiceSliceHook() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&DrawNiceSliceCallback;
    }
};

#pragma once

#include "../FuncHook.h"
#include <chrono>

class doBobHurt : public FuncHook {
   private:
    using func_t = void*(__thiscall*)(void*,glm::mat4*);
    static inline func_t oFunc;

    static void* doBobHurtCallback(void* _this, glm::mat4* matrix) {
        auto result = oFunc(_this, matrix);
        static ViewModel* viewMod = ModuleManager::getModule<ViewModel>();
        static Swing* swingMod = ModuleManager::getModule<Swing>();
        auto player = GI::getLocalPlayer();
        
        if(player && viewMod && viewMod->isEnabled()) {
            float scaleX = viewMod->mhScale.x;
            float scaleY = viewMod->mhScale.y;
            float scaleZ = viewMod->mhScale.z;
            *matrix = glm::scale(*matrix, glm::vec3(scaleX, scaleY, scaleZ));

            float rotX = glm::radians(viewMod->mhRot.x);
            float rotY = glm::radians(viewMod->mhRot.y);
            float rotZ = glm::radians(viewMod->mhRot.z);

            if(rotX != 0.0f)
                *matrix = glm::rotate(*matrix, rotX, glm::vec3(1.0f, 0.0f, 0.0f));
            if(rotY != 0.0f)
                *matrix = glm::rotate(*matrix, rotY, glm::vec3(0.0f, 1.0f, 0.0f));
            if(rotZ != 0.0f)
                *matrix = glm::rotate(*matrix, rotZ, glm::vec3(0.0f, 0.0f, 1.0f));

            float x = viewMod->mhTrans.x;
            float y = viewMod->mhTrans.y;
            float z = viewMod->mhTrans.z;
            *matrix = glm::translate(*matrix, glm::vec3(x, y, z));
        }

        if(swingMod && swingMod->shouldBlock) {
            *matrix = glm::translate(*matrix, glm::vec3(0.4, 0.0, -0.15));
            *matrix = glm::translate(*matrix, glm::vec3(-0.1f, 0.15f, -0.2f));
            *matrix = glm::translate(*matrix, glm::vec3(-0.24F, 0.25f, -0.20F));
            *matrix = glm::rotate(*matrix, -1.98F, glm::vec3(0.0F, 1.0F, 0.0F));
            *matrix = glm::rotate(*matrix, 1.30F, glm::vec3(4.0F, 0.0F, 0.0F));
            *matrix = glm::rotate(*matrix, 59.9F, glm::vec3(0.0F, 1.0F, 0.0F));
            *matrix = glm::translate(*matrix, glm::vec3(0.0f, -0.1f, 0.15f));
            *matrix = glm::translate(*matrix, glm::vec3(0.08f, 0.0f, 0.0f));
            *matrix = glm::scale(*matrix, glm::vec3(1.05f, 1.05f, 1.05f));
        }

        if(swingMod && swingMod->smallItem) {
            *matrix = glm::translate(*matrix, glm::vec3(0.5f, -0.2f, -0.6f));
        }
        
        return result;
    }

   public:
    doBobHurt() {
        OriginFunc = (void*)&oFunc;
        func = (void*)&doBobHurtCallback;
    }
};
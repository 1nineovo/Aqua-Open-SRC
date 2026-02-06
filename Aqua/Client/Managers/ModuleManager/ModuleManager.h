#pragma once
#include "Modules/ModuleBase/Module.h"
#include "Modules/Category/Client/CustomFont.h"
#include "Modules/Category/Client/ArrayList.h"
#include "Modules/Category/Client/ClickGui.h"
#include "Modules/Category/Client/Interface.h"
#include "Modules/Category/Client/Editor.h"
#include "Modules/Category/Client/Notifications.h"
#include "Modules/Category/Player/AntiCrystal.h"
#include "Modules/Category/Player/BlockReach.h"
#include "Modules/Category/Player/Offhand.h"
#include "Modules/Category/Player/Scaffold.h"
#include "Modules/Category/Player/ChestAura.h"
#include "Modules/Category/Player/ChestStealer.h"
#include "Modules/Category/Player/InvManager.h"
#include "Modules/Category/Player/PacketMine.h"
#include "Modules/Category/Player/PopCounter.h"
#include "Modules/Category/Player/Clip.h" 
#include "Modules/Category/Combat/Hitbox.h"
#include "Modules/Category/Combat/PistonCrystal.h"
#include "Modules/Category/Combat/Surround.h"
#include "Modules/Category/Combat/BowSpam.h"
#include "Modules/Category/Combat/Criticals.h"
#include "Modules/Category/Combat/Reach.h"
#include "Modules/Category/Combat/Aura.h" 
#include "Modules/Category/Combat/AutoCrystal.h"  
#include "Modules/Category/Movement/Velocity.h"
#include "Modules/Category/Movement/NoSlow.h"
#include "Modules/Category/Movement/Fly.h"
#include "Modules/Category/Movement/Phase.h"
#include "Modules/Category/Movement/Sprint.h"
#include "Modules/Category/Render/NoRender.h"
#include "Modules/Category/Render/NoSwing.h"
#include "Modules/Category/Render/NameTag.h"
#include "Modules/Category/Render/Swing.h"
#include "Modules/Category/Render/CustomFov.h"
#include "Modules/Category/Render/CameraTweaks.h"
#include "Modules/Category/Render/FullBright.h"
#include "Modules/Category/Render/ChunkBorders.h"
#include "Modules/Category/Render/NoHurtCam.h"
#include "Modules/Category/Render/BlockHighLight.h"
#include "Modules/Category/Render/SetColor.h"
#include "Modules/Category/Render/CrystalChams.h"
#include "Modules/Category/Render/PlayerCham.h"
#include "Modules/Category/Render/ViewModel.h"
#include "Modules/Category/Misc/NoPacket.h"
#include "Modules/Category/Misc/Timer.h"
#include "Modules/Category/Misc/AutoMine.h"
#include "Modules/Category/Misc/Chat.h"
class ModuleManager {
public:
	static inline std::vector<Module*> moduleList;

	static void init();
	static void shutdown();

	template <typename TRet>
	static TRet* getModule() {
		for (Module* mod : moduleList) {
			TRet* result = dynamic_cast<TRet*>(mod);
			if (result == nullptr)
				continue;
			return result;
		}
		return nullptr;
	}
	static void onKeyUpdate(int key, bool isDown);
	static void onClientTick();
	static void onNormalTick(LocalPlayer* localPlayer);
	static void onLevelTick(Level* level);
	static void onUpdateRotation(LocalPlayer* localPlayer);
	static void onSendPacket(Packet* packet);
    static void onReceivePacket(Packet* packet,bool*cancel);
    static void onImGuiRender(ImDrawList* drawlist);
	static void onD2DRender();
	static void onMCRender(MinecraftUIRenderContext* renderCtx);
	static void onLevelRender();
    static void onRenderActorBefore(Actor* actor, Vec3<float> *camera, Vec3<float> *pos);
    static void onRenderActorAfter(Actor* actor);
    static void onChestScreen(ContainerScreenController* csc);
	static void onLoadConfig(void* conf);
	static void onSaveConfig(void* conf);
};

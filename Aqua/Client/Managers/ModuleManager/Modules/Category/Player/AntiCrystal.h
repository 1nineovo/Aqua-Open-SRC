#pragma once
#pragma once
#include "../../ModuleBase/Module.h"
#include "../../../../../../SDK/NetWork/Packets/PlayerAuthInputPacket.h"

class AntiCrystal : public Module {
   private:
    float reduce = 0.6f;

   public:
    AntiCrystal();
    void onSendPacket (Packet*packet) override;
    void onReceivePacket(Packet* packet,bool* cancel) override;
};
#include "PistonCrystal.h"
#include <Minecraft/InvUtil.h>
#include <algorithm>
#include <string>

PistonCrystal::PistonCrystal()
    : Module("PistonCrystal", "Automatically places pistons and crystals", Category::COMBAT) {
    instance = this;

    registerSetting(new SliderSetting<int>("Range", "Target range", &range, 5, 1, 12));
    registerSetting(new SliderSetting<int>("Delay", "Tick delay", &delay, 3, 0, 20));
    registerSetting(new SliderSetting<int>("RotDelay", "Rotation delay", &delay_rot, 1, 0, 20));

    registerSetting(new BoolSetting("Silent", "Restore slot after place", &silent, false));
    registerSetting(new BoolSetting("Self", "Target self", &self, true));
    registerSetting(new BoolSetting("Toggle", "Disable after success", &toggle, false));
    registerSetting(new BoolSetting("UseInv", "Search whole inventory", &use_inv, false));
    registerSetting(new BoolSetting("EatStop", "Pause while eating", &eatstop, true));
    registerSetting(new BoolSetting("Render", "Render piston & redstone positions", &render, true));
}

void PistonCrystal::onEnable() {
}

void PistonCrystal::reset() {
}

bool PistonCrystal::can(const BlockPos& pos, bool crystal) {
  
}

void PistonCrystal::place(BlockPos crystal, BlockPos piston, BlockPos redstone) {
 
}

bool PistonCrystal::x_plus(BlockPos pos, bool checking) {
   
}

bool PistonCrystal::x_minus(BlockPos pos, bool checking) {
    
}

bool PistonCrystal::z_plus(BlockPos pos, bool checking) {
   
}

bool PistonCrystal::z_minus(BlockPos pos, bool checking) {
    
}

void PistonCrystal::_main(const BlockPos& pos) {
   
}

void PistonCrystal::onNormalTick(LocalPlayer* localPlayer) {
    
}

void PistonCrystal::onSendPacket(Packet* packet) {
  
}

void PistonCrystal::setAngle(Rot _mode) {
 
}

void PistonCrystal::setAngle(const BlockPos& targetPos) {
   
}

void PistonCrystal::destroy() {
}

bool PistonCrystal::check_crystal() {
  
}


bool PistonCrystal::isInRenderList(const std::vector<std::pair<BlockPos, float>>& list,
                                   const BlockPos& pos) {

}



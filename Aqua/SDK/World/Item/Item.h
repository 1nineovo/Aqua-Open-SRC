#pragma once

#include <array>
#include <string_view>

#include "../Level/Block/Block.h"
#include "ItemStack.h"

enum class SItemType {
    Helmet,
    Chestplate,
    Leggings,
    Boots,
    Sword,
    Pickaxe,
    Axe,
    Shovel,
    Food,
    None
};  // 添加 Food 枚举

class Block;
class Item {
   public:
    CLASS_MEMBER(short, mItemId, 0xAA);
    CLASS_MEMBER(int, ArmorItemType, 0x24C);
    CLASS_MEMBER(int, mProtection, 0x26C);
    CLASS_MEMBER(std::string, mName, 0xD8);
    CLASS_MEMBER(uint16_t, maxStackSize, 0xA8);
    CLASS_MEMBER(uint16_t, MaxUseDuration, 0x154);

   public:
    void setMaxDamage(int maxDamage) {
        MemoryUtil::callVirtualFunc<void>(25, this, maxDamage);
    }

    void setMaxUseDuration(int maxUseDuration) {
        MemoryUtil::callVirtualFunc<void>(26, this, maxUseDuration);
    }

    void setMaxStackSize(int maxStackSize) {
        *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xA8) = maxStackSize;
    }

    float getMaxDamage() {
        return MemoryUtil::callVirtualFunc<float>(35, this);
    }

    bool isGlint(ItemStackBase* itemStackBase) {
        return MemoryUtil::callVirtualFunc<bool, ItemStackBase*>(39, this, itemStackBase);
    }

    short getDamageValue(CompoundTag* userData) {
        using func_t = short(__thiscall*)(Item*, CompoundTag*);
        static func_t Func = reinterpret_cast<func_t>(
            MemoryUtil::findSignature("40 53 48 83 EC ? 48 8B 51 ? 33 DB 48 85 D2"));
        return Func(this, userData);
    }

    void readCompoundTag(CompoundTag* compoundTag) {
        MemoryUtil::callVirtualFunc<void>(89, this, compoundTag);
    }
    void writeCompoundTag(CompoundTag* compoundTag) {
        MemoryUtil::callVirtualFunc<void>(90, this, compoundTag);
    }

    float getDestroySpeed(ItemStackBase* item, Block* block) {
        return MemoryUtil::callVirtualFunc<float, ItemStackBase*, Block*>(81, this, item, block);
    }
    float getDestroySpeed2(ItemStackBase* item, Block* block) {
        uintptr_t** mVfTable = reinterpret_cast<
            uintptr_t**>((uintptr_t)MemoryUtil::getVtableFromSig(
            "48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C6 05 ? ? ? ? ? 48 8D 0D"));
        return MemoryUtil::callVirtualFunc<float, ItemStackBase*, uintptr_t**, Block*>(
            81, this, item, mVfTable, block);
    }

    int getArmorSlot() {
        static constexpr std::array<std::string_view, 4> armorSlots = {"_helmet", "_chestplate",
                                                                       "_leggings", "_boots"};
        for(int i = 0; i < armorSlots.size(); i++) {
            if(mName.find(armorSlots[i]) != std::string::npos) {
                return i;
            }
        }
        return -1;
    }

    bool isHelmet() {
        static constexpr std::string_view v = "_helmet";
        return mName.ends_with(v);
    }
    bool isChestplate() {
        static constexpr std::string_view v = "_chestplate";
        return mName.ends_with(v);
    }
    bool isLeggings() {
        static constexpr std::string_view v = "_leggings";
        return mName.ends_with(v);
    }
    bool isBoots() {
        static constexpr std::string_view v = "_boots";
        return mName.ends_with(v);
    }
    bool isSword() {
        static constexpr std::string_view v = "_sword";
        return mName.ends_with(v);
    }
    bool isPickaxe() {
        static constexpr std::string_view v = "_pickaxe";
        return mName.ends_with(v);
    }
    bool isAxe() {
        static constexpr std::string_view v = "_axe";
        return mName.ends_with(v);
    }
    bool isShovel() {
        static constexpr std::string_view v = "_shovel";
        return mName.ends_with(v);
    }

    // === 新增：食物检测 ===
    bool isFood() {
        // 常见食物关键字列表
        static constexpr std::array<std::string_view, 22> foodKeys = {
            "apple",        // apple, golden_apple
            "bread",        // bread
            "beef",         // raw_beef, cooked_beef
            "porkchop",     // porkchop, cooked_porkchop
            "chicken",      // chicken, cooked_chicken
            "mutton",       // mutton, cooked_mutton
            "rabbit",       // rabbit, cooked_rabbit, rabbit_stew
            "cod",          // cod, cooked_cod
            "salmon",       // salmon, cooked_salmon
            "fish",         // tropical_fish
            "potato",       // potato, baked_potato
            "carrot",       // carrot, golden_carrot
            "beetroot",     // beetroot, beetroot_soup
            "melon_slice",  // melon_slice (melon 是方块)
            "mushroom_stew",
            "soup",  // suspicious_stew, beetroot_soup
            "cookie",        "pumpkin_pie", "cake",
            "berry",  // sweet_berries, glow_berries
            "honey_bottle",
            "kelp"  // dried_kelp
        };

        for(const auto& key : foodKeys) {
            // 使用 find 进行模糊匹配
            if(mName.find(key) != std::string::npos) {
                // 如果需要排除毒马铃薯，可以解开下面注释
                // if (mName.find("poisonous") != std::string::npos) return false;
                return true;
            }
        }
        return false;
    }

    int getItemTier() {
        static constexpr std::array<std::string_view, 7> tiers = {
            "wooden_", "chainmail_", "stone_", "iron_", "golden_", "diamond_", "netherite_"};
        for(int i = 0; i < tiers.size(); i++) {
            if(mName.starts_with(tiers[i]))
                return i;
        }
        return 0;
    }

    int getArmorTier() {
        static constexpr std::array<std::string_view, 7> tiers = {
            "leather_", "chainmail_", "iron_", "golden_", "diamond_", "netherite_"};
        for(int i = 0; i < tiers.size(); i++) {
            if(mName.starts_with(tiers[i]))
                return i;
        }
        return 0;
    }

    SItemType getItemType() {
        if(isHelmet())
            return SItemType::Helmet;
        if(isChestplate())
            return SItemType::Chestplate;
        if(isLeggings())
            return SItemType::Leggings;
        if(isBoots())
            return SItemType::Boots;
        if(isSword())
            return SItemType::Sword;
        if(isPickaxe())
            return SItemType::Pickaxe;
        if(isAxe())
            return SItemType::Axe;
        if(isShovel())
            return SItemType::Shovel;

        // 在这里集成食物类型
        if(isFood())
            return SItemType::Food;

        return SItemType::None;
    }
};
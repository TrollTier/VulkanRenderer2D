//
// Created by patri on 09.07.2025.
//

#ifndef STATS_H
#define STATS_H
#include <cstdint>

enum WeaponType
{
    OneHandedSword,
    GreatSword,
    Spear,
    Bow,
    Axe
};

enum MagicElement
{
    Earth,
    Water,
    Fire,
    Wind,
    Void
};

struct Stats
{
    uint16_t baseHp;
    uint16_t currentHp;
    uint16_t physicalStrength;
    uint16_t magicStrength;
    uint16_t physicalDefense;
    uint16_t magicDefense;
    uint16_t speed;
    uint16_t agility;
};

#endif //STATS_H

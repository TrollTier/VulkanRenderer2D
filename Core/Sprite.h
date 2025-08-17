//
// Created by patri on 21.07.2025.
//

#ifndef SPRITE_H
#define SPRITE_H

#include <cstddef>

#include "SpriteFrame.h"

typedef struct
{
    size_t textureIndex{0};
    mutable uint16_t currentFrame{0};
} Sprite;

#endif //SPRITE_H

//
// Created by patri on 16.07.2025.
//

#ifndef UNIFORMBUFFEROBJECT_H
#define UNIFORMBUFFEROBJECT_H

#include <glm/glm.hpp>

#include "ImageRect.h"

struct InstanceData {
    glm::mat4 modelMatrix;  // 64 bytes  64
    ImageRect spriteFrame;  // 16 bytes  80
    uint32_t textureIndex;  //  4 bytes  84
    uint32_t _pad[3];       // 12 bytes  96
};

#endif //UNIFORMBUFFEROBJECT_H

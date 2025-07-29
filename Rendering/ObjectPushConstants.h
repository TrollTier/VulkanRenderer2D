//
// Created by patri on 16.07.2025.
//

#ifndef UNIFORMBUFFEROBJECT_H
#define UNIFORMBUFFEROBJECT_H

#include <glm/glm.hpp>

struct ObjectPushConstants {
    glm::mat4 modelMatrix;
    uint32_t textureIndex;
    uint32_t padding1;
    uint32_t padding2;
    uint32_t padding3;
};

#endif //UNIFORMBUFFEROBJECT_H

//
// Created by patri on 02.07.2025.
//

#ifndef VERTEX_H
#define VERTEX_H

#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

typedef struct {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
} Vertex;

#endif //VERTEX_H

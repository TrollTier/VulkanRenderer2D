//
// Created by patri on 22.06.2026.
//

#ifndef CIRCLE_H
#define CIRCLE_H

#include <glm/glm.hpp>

typedef struct Circle
{
    glm::vec4 color;
    glm::vec4 position;
    float radius;
    float _pad[3];
} Circle;

#endif //CIRCLE_H

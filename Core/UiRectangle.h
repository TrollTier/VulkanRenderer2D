//
// Created by patri on 12.07.2026.
//

#ifndef RECTANGLE_H
#define RECTANGLE_H
#include <glm/glm.hpp>

typedef struct
{
    glm::vec4 color;
    glm::vec4 center;
    float scaleX;
    float scaleY;
    float padding[2];
} UiRectangle;

#endif //RECTANGLE_H

//
// Created by patri on 18.09.2025.
//

#ifndef ANIMATIONDATA_H
#define ANIMATIONDATA_H

#include <cstdint>
#include <string>
#include <vector>

typedef struct
{
    uint16_t afterFrames{};
    uint8_t frame{};
} KeyFrame;

typedef struct
{
    std::string name;
    std::vector<KeyFrame> keyFrames;
    bool loops = false;
} AnimationData;

#endif //ANIMATIONDATA_H

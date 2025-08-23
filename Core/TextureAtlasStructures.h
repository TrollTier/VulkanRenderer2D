//
// Created by patri on 17.08.2025.
//

#ifndef TEXTUREATLASSTRUCTURES_H
#define TEXTUREATLASSTRUCTURES_H

#include <cstdint>
#include <string>
#include <vector>

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} AtlasFrame;

typedef struct
{
    const uint32_t id;
    const std::string filePath;
    const std::vector<AtlasFrame> frames;
} AtlasEntry;

#endif //TEXTUREATLASSTRUCTURES_H

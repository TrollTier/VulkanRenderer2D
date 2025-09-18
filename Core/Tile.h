//
// Created by patri on 25.07.2025.
//

#ifndef TILE_H
#define TILE_H

#include <string>
#include <vector>
#include "Sprite.h"

typedef struct
{
    std::string_view tileName;
    uint16_t textureAtlasEntryId;
    uint16_t frameIndex{};
    // TODO: Movement indicators etc.
} TileData;

typedef struct
{
    uint8_t layer{};
    mutable size_t tileDataIndex{};
    Sprite sprite{};
} TileLayer;

typedef struct
{
    size_t row{};
    size_t column{};
    std::vector<TileLayer> tileLayers{1};
} Tile;

#endif //TILE_H

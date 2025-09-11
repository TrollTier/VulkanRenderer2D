//
// Created by patri on 30.08.2025.
//

#ifndef TILETYPES_H
#define TILETYPES_H

#include <array>
#include "Tile.h"

constexpr std::array<TileData, 4> TileTypes
{
    {

        {
            .tileName = "default",
            .textureAtlasEntryId = 1
        },
        {
            .tileName = "wood_1",
            .textureAtlasEntryId = 4
        },
        {
            .tileName = "ground_1",
            .textureAtlasEntryId = 7
        },
        {
            .tileName = "gravel_1",
            .textureAtlasEntryId = 8
        }
    }
};

#endif //TILETYPES_H

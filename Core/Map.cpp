//
// Created by patri on 25.07.2025.
//

#include "Map.h"

#include <cmath>

Map::Map(uint16_t rows, uint16_t columns, uint16_t tileSize)
{
    m_rows = rows;
    m_columns = columns;
    m_tileSize = tileSize;

    m_tiles = std::make_unique<std::vector<Tile>>();
    m_tiles->resize(rows * columns, Tile{});

    for (size_t row = 0; row < rows; ++row)
    {
        for (size_t column = 0; column < columns; ++column)
        {
            auto& tile = m_tiles->at(column + (row * columns));
            tile.column = column;
            tile.row = row;
        }
    }
}

Map::~Map()
{
    m_tiles->clear();
}

Tile& Map::getTileAt(uint16_t column, uint16_t row) const
{
    if (!isInMap(column, row))
    {
        throw std::runtime_error("Coordinates out of map bounds");
    }

    return m_tiles->at(column + (row * m_columns));
}

bool Map::isInMap(uint16_t column, uint16_t row) const
{
    return (column < m_columns && row < m_rows);
}


const std::vector<Tile>& Map::getTiles() const
{
    return *m_tiles;
}

size_t Map::getTileSize() const
{
    return m_tileSize;
}

size_t Map::getColumns() const
{
    return m_columns;
}

size_t Map::getRows() const
{
    return m_rows;
}

void Map::setTileAt(uint16_t column, uint16_t row, uint8_t layer, size_t tileDataIndex, size_t textureIndex, uint16_t frame)
{
    if (!isInMap(column, row))
    {
        return;
    }

    auto& tile = getTileAt(column, row);

    for (int i = 0; i < tile.tileLayers.size(); i++)
    {
        auto& tileLayer = tile.tileLayers[i];

        if (tileLayer.layer == layer)
        {
            tileLayer.tileDataIndex = tileDataIndex;
            tileLayer.sprite.textureIndex = textureIndex;
            tileLayer.sprite.currentFrame = frame;

            return;
        }

        if (tileLayer.layer > layer)
        {
            tile.tileLayers.insert(
                tile.tileLayers.begin() + i,
                TileLayer
                {
                    .layer = layer,
                    .tileDataIndex = tileDataIndex,
                    .sprite = Sprite{ .textureIndex = textureIndex, .currentFrame = frame }
                });

            return;
        }
    }

    tile.tileLayers.emplace_back(
        layer,
        tileDataIndex,
        Sprite{ .textureIndex = textureIndex, .currentFrame = frame });
}






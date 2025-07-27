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

Tile &Map::getTileAt(uint16_t column, uint16_t row) const
{
    if (column < 0 || column > m_columns - 1 || row < 0 || row > m_rows - 1)
    {
        throw std::runtime_error("Coordinates out of map bounds");
    }

    return m_tiles->at(column + (row * m_columns));
}

const std::vector<Tile>& Map::getTiles() const
{
    return *m_tiles;
}

size_t Map::getTileSize() const
{
    return m_tileSize;
}




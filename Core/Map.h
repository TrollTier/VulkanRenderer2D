//
// Created by patri on 25.07.2025.
//

#ifndef MAP_H
#define MAP_H

#include <memory>
#include <vector>

#include "Tile.h"

/**
 * Map keeps track of tile information in a row and zero based single array layout.
 * [
*        0,  1,  2,  3,  4,  5,
*        6,  7,  8,  9, 10, 11,
*       12, 13, 14, 15, 16, 17,
*       18, 19, 20, 21, 22, 23,
 * ]
 * So i.e. for a 10 * 10 map, field at x = 5 = y = 2 would be at index 17
 */
class Map
{
public:
    Map(uint16_t rows, uint16_t columns, uint16_t tileSize);
    ~Map();

    [[nodiscard]] Tile& getTileAt(uint16_t column, uint16_t row) const;
    [[nodiscard]] const std::vector<Tile>& getTiles() const;
    [[nodiscard]] size_t getTileSize() const;
    [[nodiscard]] size_t getRows() const;
    [[nodiscard]] size_t getColumns() const;

private:
    uint16_t m_rows = 0;
    uint16_t m_columns = 0;
    uint16_t m_tileSize = 1;

    std::unique_ptr<std::vector<Tile>> m_tiles;
};

#endif //MAP_H

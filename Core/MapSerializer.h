//
// Created by patri on 22.10.2025.
//

#ifndef MAPSERIALIZER_H
#define MAPSERIALIZER_H

#include <filesystem>
#include <fstream>

class MapSerializer
{
public:
    static void serializeMap(const std::filesystem::path& filePath, const Map& map)
    {
        std::ofstream file;
        file.open(filePath, std::ios_base::openmode::_S_out | std::_S_trunc);

        file << 1 << std::endl;
        file << map.getColumns() << ";" << map.getRows() << ";" << map.getTileSize() << std::endl;

        for (size_t row = 0; row < map.getRows(); row++)
        {
            for (size_t column = 0; column < map.getColumns(); column++)
            {
                const auto& tile = map.getTileAt(column, row);
                file << tile.column << ";" << tile.row << ";" << tile.tileLayers.size() << ";";

                for (size_t layer = 0; layer < tile.tileLayers.size(); layer++)
                {
                    // Separator from previous layer
                    if (layer > 0)
                    {
                        file << ";";
                    }

                    const auto& tileLayer = tile.tileLayers[layer];
                    file
                        << static_cast<size_t>(tileLayer.layer) << ";"
                        << tileLayer.tileDataIndex << ";"
                        << tileLayer.sprite.textureIndex << ";"
                        << tileLayer.sprite.currentFrame;
                }

                file << std::endl;
            }
        }
    }
};

#endif //MAPSERIALIZER_H

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
    typedef struct
    {
        Map map;
        uint8_t layers;
    } DeserializeResult;

    static DeserializeResult deserializeMap(const std::filesystem::path& filePath)
    {
        std::ifstream file;
        file.open(filePath);

        std::string line;
        std::vector<CsvElement> csvElements{};
        csvElements.reserve(10);

        // read version
        std::getline(file, line);
        const uint8_t version = toUInt8(line);

        // read basic size information
        std::getline(file, line);
        fillLineElements(line, csvElements);

        uint16_t columns = toUInt16(line, csvElements[0]);
        uint16_t rows = toUInt16(line, csvElements[1]);
        uint16_t tileSize = toUInt16(line, csvElements[2]);

        Map map(rows, columns, tileSize);
        uint8_t maxLayers = 1;

        while (std::getline(file, line))
        {
            fillLineElements(line, csvElements);
            uint16_t column = toUInt16(line, csvElements[0]);
            uint16_t row = toUInt16(line, csvElements[1]);
            uint8_t layers = toUInt8(line, csvElements[2]);

            maxLayers = std::max(maxLayers, layers);

            for (uint8_t layer = 0; layer < layers; layer++)
            {
                const size_t startIndex = 3 + (layer * 4); // 4 elements per layer information
                const uint8_t writtenLayer = toUInt8(line, csvElements[startIndex]);
                const size_t tileDataIndex = toSizeT(line, csvElements[startIndex + 1]);
                const size_t textureIndex = toSizeT(line, csvElements[startIndex + 2]);
                const uint16_t currentFrame = toUInt16(line, csvElements[startIndex + 3]);

                map.setTileAt(column, row, writtenLayer, tileDataIndex, textureIndex, currentFrame);
            }
        }

        file.close();

        return DeserializeResult{map, maxLayers};
    }

    static void serializeMap(const std::filesystem::path& filePath, const Map& map)
    {
        std::ofstream file;
        file.open(filePath, std::ios_base::openmode::_S_out | std::_S_trunc);

        file << static_cast<uint8_t>(1) << std::endl;
        file << map.getColumns() << ";" << map.getRows() << ";" << map.getTileSize() << std::endl;

        for (uint16_t row = 0; row < map.getRows(); row++)
        {
            for (uint16_t column = 0; column < map.getColumns(); column++)
            {
                const auto& tile = map.getTileAt(column, row);
                file << tile.column << ";" << tile.row << ";" << tile.tileLayers.size() << ";";

                for (uint16_t layer = 0; layer < tile.tileLayers.size(); layer++)
                {
                    // Separator from previous layer
                    if (layer > 0)
                    {
                        file << ";";
                    }

                    const auto& tileLayer = tile.tileLayers[layer];
                    file
                        << static_cast<uint16_t>(tileLayer.layer) << ";"
                        << tileLayer.tileDataIndex << ";"
                        << tileLayer.sprite.textureIndex << ";"
                        << tileLayer.sprite.currentFrame;
                }

                file << std::endl;
            }
        }

        file.close();
    }

private:

    struct CsvElement
    {
        const uint16_t startIndex;
        const uint16_t endIndex;
    };

    static void fillLineElements(const std::string& line, std::vector<CsvElement>& elements)
    {
        elements.clear();

        uint16_t lastSeparatorIndex = -1;
        for (size_t i = 0; i < line.size(); i++)
        {
            if (i > UINT16_MAX)
            {
                throw std::runtime_error("Unexpected line length for texture atlas entry");
            }

            if (line[i] == ';')
            {
                elements.emplace_back(lastSeparatorIndex + 1, i - 1);
                lastSeparatorIndex = i;
            }
        }

        // Last element, no ;
        elements.emplace_back(lastSeparatorIndex + 1, line.size() - 1);
    }

    static uint8_t toUInt8(const std::string& line)
    {
        const int parsed = std::stoi(line);
        const uint8_t result(parsed);

        return result;
    }

    static uint8_t toUInt8(const std::string& line, CsvElement& element)
    {
        const std::string input = line.substr(element.startIndex, element.endIndex - element.startIndex + 1);

        const int parsed = std::stoi(input);
        const uint8_t result(parsed);

        return result;
    }

    static uint16_t toUInt16(const std::string& line, CsvElement& element)
    {
        const std::string input = line.substr(element.startIndex, element.endIndex - element.startIndex + 1);

        const int parsed = std::stoi(input);
        const uint16_t result(parsed);

        return result;
    }

    static size_t toSizeT(const std::string& line, CsvElement& element)
    {
        const std::string input = line.substr(element.startIndex, element.endIndex - element.startIndex + 1);

        const int parsed = std::stoi(input);
        const size_t result(parsed);

        return result;
    }
};

#endif //MAPSERIALIZER_H

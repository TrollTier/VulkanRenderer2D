//
// Created by patri on 17.08.2025.
//

#ifndef TEXTUREATLASPARSER_H
#define TEXTUREATLASPARSER_H

#include <filesystem>
#include <vector>
#include <fstream>
#include <array>

#include "TextureAtlasStructures.h"

class TextureAtlasParser
{
public:
    static std::vector<AtlasEntry> parseAtlas(const std::filesystem::path& file)
    {
        std::vector<AtlasEntry> result{};
        std::ifstream stream(file, std::_S_in);

        if (!stream.is_open())
        {
            return result;
        }

        std::string line;
        while (std::getline(stream, line))
        {
            if (line.empty() || line.starts_with("#"))
            {
                continue;
            }

            result.push_back(parseAtlasEntry(line));
        }

        return result;
    }

private:
    struct CsvElement
    {
        const uint16_t startIndex;
        const uint16_t endIndex;
    };

    static AtlasEntry parseAtlasEntry(std::string& line)
    {
        std::vector<CsvElement> elements{};

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

        const uint32_t id = toUInt32(line, elements[0]);
        const std::string fileName = toString(line, elements[1]);
        const uint16_t framesCount = toUInt16(line, elements[2]);

        std::vector<AtlasFrame> frames{framesCount};

        for (uint16_t i = 0; i < framesCount; i++)
        {
            const uint16_t frameParametersOffset = 3 + (i * 5); // Frames offset + number of elements per frame
            const uint16_t frameIndex = toUInt16(line, elements[frameParametersOffset]);

            std::array<uint16_t, 4> frameBounds{};

            for (uint16_t paramIndex = 0; paramIndex < 4; paramIndex++)
            {
                frameBounds[paramIndex] = toUInt16(line, elements[frameParametersOffset + 1 + paramIndex]);
            }

            frames[frameIndex].x = frameBounds[0];
            frames[frameIndex].y = frameBounds[1];
            frames[frameIndex].width = frameBounds[2];
            frames[frameIndex].height = frameBounds[3];
        }

        AtlasEntry entry
        {
            id,
            fileName,
            std::move(frames)
        };

        return std::move(entry);
    }

    static std::string toString(const std::string& line, CsvElement element)
    {
        return line.substr(element.startIndex, element.endIndex - element.startIndex + 1);
    }

    static uint16_t toUInt16(const std::string& line, CsvElement& element)
    {
        const std::string input = line.substr(element.startIndex, element.endIndex - element.startIndex + 1);

        const int parsed = std::stoi(input);
        const uint16_t result(parsed);

        return result;
    }

    static uint32_t toUInt32(const std::string& line, CsvElement& element)
    {
        const std::string input = line.substr(element.startIndex, element.endIndex - element.startIndex + 1);

        const int parsed = std::stoi(input);
        const uint32_t result(parsed);

        return result;
    }
};

#endif //TEXTUREATLASPARSER_H

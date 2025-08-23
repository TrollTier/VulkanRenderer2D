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
    static AtlasEntry parseAtlasEntry(std::string& line)
    {
        const size_t idSeparator = line.find(';');
        const std::string idInput = line.substr(0, idSeparator);
        const uint32_t id = toUInt32(idInput);

        const size_t nameSeparator = line.find(';', idSeparator + 1);
        const std::string fileName = line.substr(idSeparator + 1, nameSeparator - idSeparator - 1);

        const size_t framesCountSeparator = line.find(';', nameSeparator + 1);
        const std::string framesCountInput = line.substr(nameSeparator + 1, framesCountSeparator - nameSeparator - 1);
        const uint16_t framesCount = toUInt16(framesCountInput);

        std::vector<AtlasFrame> frames{framesCount};
        size_t currentSeparator = framesCountSeparator;

        for (uint16_t i = 0; i < framesCount; i++)
        {
            const uint16_t frameIndexSeparator = line.find(';', currentSeparator + 1);
            const std::string frameIndexInput = line.substr(currentSeparator + 1, frameIndexSeparator - currentSeparator - 1);
            const uint16_t frameIndex = toUInt16(frameIndexInput);
            currentSeparator = frameIndexSeparator;

            std::array<uint16_t, 4> frameBounds{};

            for (uint16_t paramIndex = 0; paramIndex < 4; paramIndex++)
            {
                const uint16_t parameterSeparatorIndex = line.find(';', currentSeparator + 1);
                const std::string parameterInput = line.substr(currentSeparator + 1, parameterSeparatorIndex - currentSeparator - 1);

                frameBounds[paramIndex] = toUInt16(parameterInput);
                currentSeparator = parameterSeparatorIndex;
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

    static uint16_t toUInt16(const std::string& input)
    {
        const int parsed = std::stoi(input);
        const uint16_t result(parsed);

        return result;
    }

    static uint32_t toUInt32(const std::string& input)
    {
        const int parsed = std::stoi(input);
        const uint32_t result(parsed);

        return result;
    }
};

#endif //TEXTUREATLASPARSER_H

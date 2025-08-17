//
// Created by patri on 17.07.2025.
//

#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#define STB_IMAGE_IMPLEMENTATION
#include <filesystem>
#include <stdexcept>

#include "../include/stb/stb_image.h"

typedef struct
{
    int width;
    int height;
    int channels;
    stbi_uc* data;
} ImageInfo;

class ImageLoader
{
public:
    static ImageInfo loadImage(const std::filesystem::path& imagePath)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(imagePath.string().data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }

        return ImageInfo
        {
            texWidth,
            texHeight,
            texChannels,
            pixels
        };
    }
};

#endif //IMAGELOADER_H

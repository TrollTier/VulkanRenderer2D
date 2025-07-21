//
// Created by patri on 21.07.2025.
//

#include "MeshLoader.h"

MeshLoader::MeshLoader()
{
    const std::vector<Vertex> vertices = {
        {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    m_quad = std::make_shared<Mesh>(0, vertices, indices);
}

Mesh MeshLoader::loadMesh(const char *assetPath)
{
    throw std::runtime_error("Not implemented");
}

std::weak_ptr<Mesh> MeshLoader::getQuadMesh() const
{
    return m_quad;
}

const std::shared_ptr<Mesh> & MeshLoader::getQuadMeshShared() const
{
    return m_quad;
}

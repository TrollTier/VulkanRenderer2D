//
// Created by patri on 16.07.2025.
//

#include "Mesh.h"

Mesh::Mesh(size_t meshIndex, std::vector<Vertex> vertices, std::vector<uint16_t> indices)
{
    m_meshIndex = meshIndex;
    m_vertices = std::move(vertices);
    m_indices = std::move(indices);
}

const std::vector<Vertex>& Mesh::getVertices() const
{
    return m_vertices;
}

const std::vector<uint16_t>& Mesh::getIndices() const
{
    return m_indices;
}

size_t Mesh::getMeshIndex() const
{
    return m_meshIndex;
}


//
// Created by patri on 16.07.2025.
//

#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint16_t> indices)
{
    m_vertices = vertices;
    m_indices = indices;
}

const std::vector<Vertex>& Mesh::getVertices() const
{
    return m_vertices;
}

const std::vector<uint16_t>& Mesh::getIndices() const
{
    return m_indices;
}

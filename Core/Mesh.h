//
// Created by patri on 16.07.2025.
//

#ifndef MESH_H
#define MESH_H
#include <vector>

#include "Vertex.h"

class Mesh
{
public:
    Mesh(size_t meshIndex, std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    [[nodiscard]] const std::vector<Vertex>& getVertices() const;
    [[nodiscard]] const std::vector<uint16_t>& getIndices() const;
    [[nodiscard]] size_t getMeshIndex() const;

private:
    size_t m_meshIndex;
    std::vector<Vertex> m_vertices;
    std::vector<uint16_t> m_indices;
};

#endif //MESH_H

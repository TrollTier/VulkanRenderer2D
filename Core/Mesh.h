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
    Mesh(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    const std::vector<Vertex>& getVertices() const;
    const std::vector<uint16_t>& getIndices() const;

private:
    std::vector<Vertex> m_vertices;
    std::vector<uint16_t> m_indices;
};

#endif //MESH_H

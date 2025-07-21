//
// Created by patri on 21.07.2025.
//

#ifndef MESHLOADER_H
#define MESHLOADER_H
#include <memory>

#include "Mesh.h"

class MeshLoader
{
public:
    MeshLoader();

    static Mesh loadMesh(const char* assetPath);

    [[nodiscard]] std::weak_ptr<Mesh> getQuadMesh() const;
    [[nodiscard]] const std::shared_ptr<Mesh>& getQuadMeshShared() const;
private:
    std::shared_ptr<Mesh> m_quad;
};

#endif //MESHLOADER_H

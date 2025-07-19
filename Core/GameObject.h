//
// Created by patri on 18.07.2025.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <memory>
#include <glm/glm.hpp>
#include <utility>

#include "Mesh.h"

class GameObject
{
public:
    GameObject(glm::vec3 worldPosition, std::weak_ptr<Mesh> mesh);

    [[nodiscard]] const glm::vec3& getWorldPosition() const;
    [[nodiscard]] std::weak_ptr<Mesh> getMesh() const;

private:
    glm::vec3 m_worldPosition{};
    std::weak_ptr<Mesh> m_mesh;
};

#endif //GAMEOBJECT_H

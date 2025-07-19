//
// Created by patri on 18.07.2025.
//

#include "GameObject.h"

GameObject::GameObject(glm::vec3 worldPosition, std::weak_ptr<Mesh> mesh)
{
    m_worldPosition = std::move(worldPosition);
    m_mesh = std::move(mesh);
}


const glm::vec3& GameObject::getWorldPosition() const
{
    return m_worldPosition;
}

std::weak_ptr<Mesh> GameObject::getMesh() const
{
    return m_mesh;
}
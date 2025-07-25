//
// Created by patri on 18.07.2025.
//

#include "GameObject.h"

GameObject::GameObject(size_t index, glm::vec3 worldPosition, std::weak_ptr<Mesh> mesh, Sprite sprite)
{
    m_worldPosition = worldPosition;
    m_mesh = std::move(mesh);
    m_index = index;
    m_sprite = sprite;
}


const glm::vec3& GameObject::getWorldPosition() const
{
    return m_worldPosition;
}

std::weak_ptr<Mesh> GameObject::getMesh() const
{
    return m_mesh;
}

size_t GameObject::getIndex() const
{
    return m_index;
}

const Sprite &GameObject::getSprite() const
{
    return m_sprite;
}

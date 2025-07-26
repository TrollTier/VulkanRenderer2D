//
// Created by patri on 18.07.2025.
//

#include "GameObject.h"

GameObject::GameObject(size_t index, glm::vec3 worldPosition, size_t meshHandle, Sprite sprite)
{
    m_worldPosition = worldPosition;
    m_meshHandle = meshHandle;
    m_index = index;
    m_sprite = sprite;
}


const glm::vec3& GameObject::getWorldPosition() const
{
    return m_worldPosition;
}

size_t GameObject::getMeshHandle() const
{
    return m_meshHandle;
}

size_t GameObject::getIndex() const
{
    return m_index;
}

const Sprite &GameObject::getSprite() const
{
    return m_sprite;
}

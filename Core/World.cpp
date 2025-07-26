//
// Created by patri on 24.07.2025.
//

#include "World.h"

World::~World()
{
    m_gameObjects.clear();
}

const GameObject& World::addGameObject(glm::vec3 worldPosition, size_t meshHandle, Sprite sprite)
{
    const size_t index = m_gameObjects.size();
    m_gameObjects.emplace_back(index, worldPosition, meshHandle, sprite);

    return m_gameObjects[index];
}

const std::vector<GameObject>& World::getGameObjects() const
{
    return m_gameObjects;
}



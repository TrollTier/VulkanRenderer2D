//
// Created by patri on 24.07.2025.
//

#include "World.h"

World::~World()
{
    m_gameObjects.clear();
}

const GameObject& World::addGameObject(glm::vec3 worldPosition, std::weak_ptr<Mesh> mesh, Sprite sprite)
{
    const size_t index = m_gameObjects.size();
    m_gameObjects.emplace_back(index, worldPosition, mesh, sprite);

    return m_gameObjects[index];
}

const std::vector<GameObject>& World::getGameObjects() const
{
    return m_gameObjects;
}



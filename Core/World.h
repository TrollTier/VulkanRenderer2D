//
// Created by patri on 24.07.2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "GameObject.h"

class World
{
public:
    ~World();

    const GameObject& addGameObject(glm::vec3 worldPosition, std::weak_ptr<Mesh> mesh, Sprite sprite);
    const std::vector<GameObject>& getGameObjects() const;

private:
    std::vector<GameObject> m_gameObjects;
};

#endif //WORLD_H

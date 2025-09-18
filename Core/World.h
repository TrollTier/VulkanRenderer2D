//
// Created by patri on 24.07.2025.
//

#ifndef WORLD_H
#define WORLD_H
#include <optional>

#include "AnimationSystem.h"
#include "GameObject.h"

class World
{
public:
    World();
    ~World();

    const GameObject& addGameObject(
        glm::vec3 worldPosition,
        size_t meshHandle,
        Sprite sprite,
        std::optional<size_t> animatorIndex);
    [[nodiscard]] const std::vector<GameObject>& getGameObjects() const;
    [[nodiscard]] AnimationSystem& getAnimationSystem() const;

private:
    std::vector<GameObject> m_gameObjects;
    std::unique_ptr<AnimationSystem> m_animationSystem;
};

#endif //WORLD_H

//
// Created by patri on 18.07.2025.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <memory>
#include <glm/glm.hpp>
#include <utility>

#include "Mesh.h"
#include "Sprite.h"

class GameObject
{
public:
    GameObject(size_t index, glm::vec3 worldPosition, size_t meshHandle, Sprite sprite);

    [[nodiscard]] const glm::vec3& getWorldPosition() const;
    [[nodiscard]] size_t getMeshHandle() const;
    [[nodiscard]] size_t getIndex() const;
    [[nodiscard]] const Sprite& getSprite() const;

    void moveBy(const glm::vec3& delta)
    {
        m_worldPosition += delta;
    }

private:
    size_t m_index{};
    glm::vec3 m_worldPosition{};
    size_t m_meshHandle{};
    Sprite m_sprite;
};

#endif //GAMEOBJECT_H

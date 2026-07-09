//
// Created by patri on 23.11.2025.
//

#ifndef GAME_H
#define GAME_H

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "TextureAtlasStructures.h"
#include "GLFW/glfw3.h"
#include "Camera.h"
#include "Input.h"
#include "Map.h"
#include "WindowContext.h"
#include "World.h"
#include "../Rendering/VulkanRenderer.h"
#include "../Rendering/SpriteRenderData.h"
#include <glm/gtc/matrix_transform.hpp>

class Camera;
class Map;
class World;
class VulkanRenderer;
class VulkanResources;
class VulkanWindow;

class Game {
public:
    const float ZOOM_STEP_FACTOR = 1.0f;
    const uint8_t FRAMES_PER_SECOND = 60;
    const float SECONDS_PER_FRAME = 1.0f / (float)FRAMES_PER_SECOND;

    int32_t PIXELS_PER_UNIT = 64;

    Game();

    void RunLoop();

private:
    std::vector<AtlasEntry> m_atlasEntries;

    GLFWwindow* m_window;
    std::shared_ptr<VulkanWindow> m_vulkanWindow;
    std::shared_ptr<VulkanResources> m_vulkanResources;
    std::unique_ptr<VulkanRenderer> m_renderer;
    std::unique_ptr<World> m_world;
    std::unique_ptr<Map> m_map;
    std::vector<size_t> m_textureIndices;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Input> m_inputSystem;
    std::unique_ptr<WindowContext> m_windowContext;

    std::weak_ptr<ObjectBuffer<SpriteRenderData>> m_spriteBuffer;
    std::weak_ptr<ObjectBuffer<Circle>> m_circles;

    int32_t m_selectedGameObjectIndex = -1;

    void drawMap();
    void drawSelectedCharacter();

    [[nodiscard]] glm::vec2 screenToWorld(const glm::vec2& screenPos) const;
    [[nodiscard]] glm::vec3 mouseToWorld() const;

    void drawSprite(
        size_t objectIndex,
        const glm::vec3& worldPosition,
        const glm::vec3& scale,
        const Sprite& sprite,
        const glm::vec3& cameraOffset)
    {
        const auto spriteBuffer = m_spriteBuffer.lock();
        if (!spriteBuffer)
        {
            return;
        }

        const glm::vec3 screenPosition = glm::vec3(
             (static_cast<float>(worldPosition.x) - cameraOffset.x) * static_cast<float>(m_renderer->getPixelsPerUnit()),
             (static_cast<float>(worldPosition.y) - cameraOffset.y) * static_cast<float>(m_renderer->getPixelsPerUnit()),
             worldPosition.z);

        const auto& texture = m_renderer->getTexture(sprite.textureIndex);
        auto& spriteObject = spriteBuffer->m_data[objectIndex];

        spriteObject.modelMatrix =
            glm::translate(glm::mat4(1.0f), screenPosition) *
            glm::scale(glm::mat4(1), scale);
        spriteObject.spriteFrame = texture.getFrame(sprite.currentFrame);
        spriteObject.textureIndex = sprite.textureIndex;

        spriteBuffer->m_dataSize = std::max(objectIndex + 1, spriteBuffer->m_dataSize);
    }
};



#endif //GAME_H

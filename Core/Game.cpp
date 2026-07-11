//
// Created by patri on 23.11.2025.
//

#include "Game.h"

#include <iostream>

#include "Input.h"
#include "MapSerializer.h"
#include "UiRectangle.h"
#include "../Rendering/VulkanRenderer.h"

Game::Game()
{
	glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(1280, 768, "Vulkan API Tutorial", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(m_window, nullptr);
    glfwMaximizeWindow(m_window);

    m_vulkanWindow = std::make_shared<VulkanWindow>(m_window);

    std::vector<const char*> instanceExtensions{};
    m_vulkanWindow->fillRequiredInstanceExtensions(instanceExtensions);
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    m_vulkanResources = std::make_shared<VulkanResources>(m_vulkanWindow);
    m_vulkanResources->initialize(
        true,
        m_validationLayers,
        instanceExtensions);

    const std::filesystem::path assetsBasePath = std::filesystem::path("..") / "Assets";

    m_renderer = std::make_unique<VulkanRenderer>(
        assetsBasePath,
        m_vulkanResources,
        PIXELS_PER_UNIT);
    m_renderer->initialize();

	m_spriteBuffer = m_renderer->registerDataType<SpriteRenderData>(10000);
	m_circles = m_renderer->registerDataType<Circle>(100);
	m_rectangles = m_renderer->registerDataType<UiRectangle>(100);

    m_atlasEntries = TextureAtlasParser::parseAtlas(assetsBasePath / "Textures/textures.atlas");

    for (const auto& atlas: m_atlasEntries)
    {
        m_textureIndices.push_back(m_renderer->loadTexture(atlas));
    }

    m_world = std::make_unique<World>();

    auto& animationSystem = m_world->getAnimationSystem();
    animationSystem.addAnimationData(
    {
        .name = "open_treasure",
        .keyFrames =
        {
            KeyFrame{.afterFrames = 0, .frame = 0 },
            KeyFrame{.afterFrames = 100, .frame = 1 },
            KeyFrame{.afterFrames = 100, .frame = 0 }
        },
        .loops = false
    });

	animationSystem.addAnimationData(
		{
		.name = "treasure_idle_closed",
		.keyFrames =
		{
			KeyFrame { .afterFrames = 0, .frame = 0 }
		}
		});
	animationSystem.addAnimator(Animator(animationSystem.getAnimationDataIndexByName("treasure_idle_closed")));

    animationSystem.addAnimationData(
    {
        .name = "blob_idle",
        .keyFrames =
        {
            KeyFrame{.afterFrames = 0, .frame = 0 },
            KeyFrame{.afterFrames = 30, .frame = 1 },
            KeyFrame{.afterFrames = 30, .frame = 2 },
            KeyFrame{.afterFrames = 30, .frame = 3 },
            KeyFrame{.afterFrames = 30, .frame = 0 },
        },
        .loops = true
    });
    animationSystem.addAnimator(Animator(animationSystem.getAnimationDataIndexByName("blob_idle")));

    m_world->addGameObject(
            { 30, 30, 1},
            0,
            Sprite{.textureIndex = 8},
            0);

    m_world->addGameObject(
        { 29, 30, 1},
        0,
        Sprite{.textureIndex = 9},
        1);

	const auto filePath = std::filesystem::path( assetsBasePath / "Maps" / "Level1.fecmap");
	const auto result = MapSerializer::deserializeMap(filePath);
    m_map = std::make_unique<Map>(result.map);

    const auto windowExtent = m_vulkanWindow->getWindowExtent();

    const CameraArea visibleArea
    {
        static_cast<float>(windowExtent.width) / static_cast<float>(PIXELS_PER_UNIT),
        static_cast<float>(windowExtent.height) / static_cast<float>(PIXELS_PER_UNIT),
        1.0f,
        10.0f
    };

    m_camera = std::make_unique<Camera>(
        glm::vec3((float)m_map->getColumns() / 2.0f, (float)m_map->getRows() / 2.0f, 0.0f),
        visibleArea,
        windowExtent.width,
        windowExtent.height);

	m_inputSystem = std::make_unique<Input>();
	m_inputSystem->init(m_window);

	const std::function clickLambda = [this](const MouseClickEvent& data)
	{
		switch (data.button)
		{
			case MouseButton::Left:
			{
				m_selectedGameObjectIndex = -1;

				const auto& objects = m_world->getGameObjects();
				for (size_t i = 0; i < objects.size(); i++)
				{
					const auto& object = objects[i];
					const auto& worldPos = object.getWorldPosition();
					const auto& sprite = object.getSprite();
					const auto& texture = m_atlasEntries[sprite.textureIndex];
					const auto& frame = texture.frames[sprite.currentFrame];

					const auto mouseWorldPos = screenToWorld(glm::vec3(data.x, data.y, 0));
					if (worldPos.x <= mouseWorldPos.x &&
						worldPos.y <= mouseWorldPos.y &&
						mouseWorldPos.x <= worldPos.x + frame.width &&
						mouseWorldPos.y <= worldPos.y + frame.height)
					{
						m_selectedGameObjectIndex = static_cast<size_t>(i);
						return;
					}
				}
				break;
			}

			case MouseButton::Right:
			{
				if (m_selectedGameObjectIndex < 0)
				{
					return;
				}

				const auto mouseWorldPos = screenToWorld(glm::vec3(data.x, data.y, 0));
				auto& object = m_world->getGameObject(m_selectedGameObjectIndex);

				const auto positionInGrid = glm::vec3(
					glm::floor(mouseWorldPos.x),
					glm::floor(mouseWorldPos.y),
					0);

				if (m_map->isInMap(
						static_cast<uint16_t>(positionInGrid.x),
						static_cast<uint16_t>(positionInGrid.y)))
				{
					object.setWorldPosition(positionInGrid);
				}

				break;
			}

			case MouseButton::Middle:
			{
				const auto x = static_cast<float>(data.x) / m_vulkanWindow->getWindowExtent().width;
				const auto y = static_cast<float>(data.y) / m_vulkanWindow->getWindowExtent().height;

				if (m_circleIsNext)
				{
					m_renderer->getDataBuffer<Circle>(m_circles).append(
						Circle(
							glm::vec4(255, 0, 0, 1),
							glm::vec4(x, y, 0, 0),
							0.05));
				}
				else
				{
					m_renderer->getDataBuffer<UiRectangle>(m_rectangles).append(
						UiRectangle(
						glm::vec4(0, 0, 255, 1),
						glm::vec4(x, y, 0, 0),
						0.025f,
						0.025f));
				}
				m_circleIsNext = !m_circleIsNext;
				break;
			}

			default: break;
		}
	};
	m_inputSystem->onClick(clickLambda);

	m_windowContext = std::make_unique<WindowContext>(m_window, m_inputSystem.get());
	glfwSetWindowUserPointer(m_window, m_windowContext.get());
}

void Game::RunLoop()
{
	auto startOfLastUpdate = std::chrono::high_resolution_clock::now();
	float secondsSinceLastUpdate = 0.0f;

	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();

		const auto startOfFrame = std::chrono::high_resolution_clock::now();
		const auto startOfCurrentUpdate = std::chrono::high_resolution_clock::now();

		const std::chrono::duration<float, std::milli> durationSinceLastUpdate =
			startOfCurrentUpdate - startOfLastUpdate;

		const Timestep step
		{
			durationSinceLastUpdate.count(),
			durationSinceLastUpdate.count() / 1000
		};

		secondsSinceLastUpdate += step.deltaSeconds;
		std::cout << "Seconds since last update: " << secondsSinceLastUpdate << std::endl;

		if (secondsSinceLastUpdate >= SECONDS_PER_FRAME)
		{
			m_world->getAnimationSystem().update(step);
			secondsSinceLastUpdate = 0.0f;
		}

		startOfLastUpdate = startOfCurrentUpdate;

		const auto startOfRender = std::chrono::high_resolution_clock::now();

		draw();

		const auto endOfRender = std::chrono::high_resolution_clock::now();

		const auto endOfFrame = std::chrono::high_resolution_clock::now();
		const auto frameDuration = endOfFrame - startOfFrame;
		const auto renderDuration = endOfRender - startOfRender;

		std::cout << "Frame:" << std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count() << std::endl;
		std::cout << "Render:" << std::chrono::duration_cast<std::chrono::milliseconds>(renderDuration).count() << std::endl;
	}
}

void Game::draw()
{
	m_drawRequests.clear();

	const auto& frustum = m_camera->getFrustum();
	const auto offset = glm::vec3(frustum.x, frustum.y, 0);

	const auto& tiles = m_map->getTiles();
	const auto& gameObjects = m_world->getGameObjects();

	const glm::vec3 scale{PIXELS_PER_UNIT, PIXELS_PER_UNIT, 1 };

	size_t objectIndex = 0;
	uint8_t maxMapLayer = 0;

	for (const auto& tile : tiles)
	{
		if (static_cast<float>(tile.column + 1) < frustum.x || static_cast<float>(tile.column) > frustum.toX ||
			static_cast<float>(tile.row + 1) < frustum.y || static_cast<float>(tile.row) > frustum.toY)
		{
			continue;
		}

		const glm::vec3 worldPos = glm::vec3(tile.column, tile.row, 1);

		for (const auto& layer : tile.tileLayers)
		{
			drawSprite(objectIndex, layer.layer, worldPos, scale, layer.sprite, offset);
			objectIndex++;
			maxMapLayer = glm::max(maxMapLayer, layer.layer);
		}
	}

	const size_t gameObjectsLayer = maxMapLayer + 1;

	for (const auto& gameObject : gameObjects)
	{
		const auto worldPosition = gameObject.getWorldPosition();

		if ((worldPosition.x + 1) < frustum.x || worldPosition.x > frustum.toX ||
			(worldPosition.y + 1) < frustum.y || worldPosition.y > frustum.toY)
		{
			continue;
		}

		if (gameObject.getAnimatorIndex().has_value())
		{
			const auto animatorIndex = gameObject.getAnimatorIndex().value();
			const auto& animator = m_world->getAnimationSystem().getAnimator(animatorIndex);
			const auto& animationData =
				m_world->getAnimationSystem().getAnimationData(animator.m_animationDataIndex);

			const Sprite sprite = Sprite
			{
				.textureIndex = gameObject.getSprite().textureIndex,
				.currentFrame = animationData->keyFrames[animator.m_currentKeyFrame].frame
			};

			drawSprite(objectIndex,gameObjectsLayer, worldPosition, scale, sprite, offset);
			objectIndex++;
		}
		else
		{
			drawSprite(objectIndex,gameObjectsLayer, worldPosition, scale, gameObject.getSprite(), offset);
			objectIndex++;
		}
	}

	const auto& circleBuffer = m_renderer->getDataBuffer<Circle>(m_circles);
	for (size_t i = 0; i < circleBuffer.m_dataSize; i++)
	{
		m_drawRequests.emplace_back(
			m_circles,
			1,
			i,
			CIRCLE_LAYER,
			i);
	}

	const auto& rectBuffer = m_renderer->getDataBuffer<UiRectangle>(m_rectangles);
	for (size_t i = 0; i < rectBuffer.m_dataSize; i++)
	{
		m_drawRequests.emplace_back(
			m_rectangles,
			2,
			i,
			CIRCLE_LAYER,
			i);
	}

	m_renderer->drawScene(*m_camera, m_drawRequests, nullptr);
}

void Game::drawSelectedCharacter()
{
	if (m_selectedGameObjectIndex < 0)
	{
		return;
	}

	const auto& objects = m_world->getGameObject(m_selectedGameObjectIndex);
}

glm::vec2 Game::screenToWorld(const glm::vec2& screenPos) const
{
	const auto& frustum = m_camera->getFrustum();
	const auto x = screenPos.x / static_cast<float>(PIXELS_PER_UNIT) + frustum.x;
	const auto y = screenPos.y / static_cast<float>(PIXELS_PER_UNIT) + frustum.y;

	return { x, y };
}

glm::vec3 Game::mouseToWorld() const
{
	double xpos, ypos;
	glfwGetCursorPos(m_window, &xpos, &ypos);

	return glm::vec3(screenToWorld({xpos, ypos}), 0);
}

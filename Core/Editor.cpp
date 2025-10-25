//
// Created by patri on 24.07.2025.
//

#include "Editor.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <chrono>

#include <iostream>
#include <random>
#include <thread>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "TextureAtlasParser.h"
#include "TileTypes.h"
#include "Timestep.h"
#include "../Rendering/VulkanRenderer.h"

#include "../Core/World.h"
#include <windows.h>

#include "MapSerializer.h"

Editor::Editor()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(1280, 768, "Vulkan API Tutorial", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
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

	const std::filesystem::path assetsBasePath = "../Assets";

    m_renderer = std::make_unique<VulkanRenderer>(
		assetsBasePath,
    	m_vulkanResources,
    	PIXELS_PER_UNIT);
    m_renderer->initialize();

	initImGui();

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
		.loops = true
	});
	animationSystem.addAnimator(Animator(animationSystem.getAnimationDataIndexByName("open_treasure")));

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

    m_map = std::make_unique<Map>(50, 50, 1);

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

    glfwSetMouseButtonCallback(m_window, glfwMouseButtonHandler);
	glfwSetWindowSizeCallback(m_window, glfwWindowResize);
	glfwSetScrollCallback(m_window, glfwScrollCallback);
}

void Editor::initImGui()
{
    VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(poolSizes);
	pool_info.pPoolSizes = poolSizes;

	if (vkCreateDescriptorPool(m_vulkanResources->m_logicalDevice, &pool_info, nullptr, &m_imGuiPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor pool for ImGui");
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	VkPipelineRenderingCreateInfoKHR info{VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
	info.pColorAttachmentFormats = &m_renderer->getSwapchain().m_format.format;
	info.colorAttachmentCount = 1;

	const size_t imageCount = m_renderer->getSwapchain().getImageCount();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(m_window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.ApiVersion = VK_API_VERSION_1_3;
	init_info.Instance = m_vulkanResources->m_instance;
	init_info.PhysicalDevice = m_vulkanResources->m_physicalDevice;
	init_info.Device = m_vulkanResources->m_logicalDevice;
	init_info.QueueFamily = m_vulkanResources->m_graphicsQueueFamilyIndex;
	init_info.Queue = m_vulkanResources->m_graphicsQueue;
	init_info.DescriptorPool = m_imGuiPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = imageCount;
	init_info.ImageCount = imageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.RenderPass = nullptr;
	init_info.UseDynamicRendering = true;
	init_info.PipelineRenderingCreateInfo = info;
	ImGui_ImplVulkan_Init(&init_info);
}

void Editor::RunLoop()
{
    auto startOfLastUpdate = std::chrono::high_resolution_clock::now();
	float secondsSinceLastUpdate = 0.0f;

	while (!glfwWindowShouldClose(m_window))
    {
		glfwPollEvents();

        const auto startOfFrame = std::chrono::high_resolution_clock::now();

        auto startOfCurrentUpdate = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float, std::milli> durationSinceLastUpdate =
            startOfCurrentUpdate - startOfLastUpdate;

        const Timestep step
        {
            durationSinceLastUpdate.count(),
            durationSinceLastUpdate.count() / 1000
        };

		startOfLastUpdate = startOfCurrentUpdate;
		secondsSinceLastUpdate += step.deltaSeconds;
		std::cout << "Seconds since last update: " << secondsSinceLastUpdate << std::endl;

		if (secondsSinceLastUpdate >= SECONDS_PER_FRAME)
		{
			updateAnimations(step);
			secondsSinceLastUpdate = 0.0f;
		}

		handleKeyInput(step);
		setSelectedTile();

        startOfLastUpdate = startOfCurrentUpdate;

        const auto startOfRender = std::chrono::high_resolution_clock::now();

		drawMap();
		updateUI();

    	ImDrawData* uiData = ImGui::GetDrawData();
    	m_renderer->drawScene(*m_camera, uiData);

        const auto endOfRender = std::chrono::high_resolution_clock::now();

        const auto endOfFrame = std::chrono::high_resolution_clock::now();
        const auto frameDuration = endOfFrame - startOfFrame;
        const auto renderDuration = endOfRender - startOfRender;

        std::cout << "Frame:" << std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count() << std::endl;
        std::cout << "Render:" << std::chrono::duration_cast<std::chrono::milliseconds>(renderDuration).count() << std::endl;
    }

	vkDeviceWaitIdle(m_vulkanResources->m_logicalDevice);
	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(m_vulkanResources->m_logicalDevice, m_imGuiPool, nullptr);
}

void Editor::updateAnimations(const Timestep &step)
{
	if (!m_runAnimations)
	{
		return;
	}

	m_world->getAnimationSystem().update(step);
}

void Editor::updateUI()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Stats", &m_showImGui, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Selected tile: %d", m_selectedTileType);
	ImGui::Text("Selected tile frame: %d", m_selectedFrame);
	ImGui::Text("Selected layer: %d", m_selectedLayer);

	if (ImGui::Button("Save"))
	{
		saveMap();
	}

	ImGui::SameLine();

	if (ImGui::Button("Open"))
	{
		openMap();
	}

	if (ImGui::Button("Animations"))
	{
		m_runAnimations = !m_runAnimations;
	}

	if (ImGui::Button("Layer 1"))
	{
		m_selectedLayer = 0;
	}

	for (uint8_t i = 1; i < m_layerCount; i++)
	{
		ImGui::SameLine();

		if (ImGui::Button("Layer " + (i + 1)))
		{
			m_selectedLayer = i;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Add layer") && m_layerCount < UINT8_MAX - 1)
	{
		m_layerCount += 1;
		m_selectedLayer = m_layerCount - 1;
	}

	for (size_t i = 0; i < TileTypes.size(); i++)
	{
		if (ImGui::Button(TileTypes[i].tileName.data()))
		{
			m_selectedTileType = static_cast<int32_t>(i);
			m_selectedFrame = 0;
		}
	}

	ImGui::End();
	ImGui::Render();
}

void Editor::setSelectedTile()
{
	if (m_selectedTileType < 0 || m_selectedFrame < 0 || ImGui::IsWindowHovered((ImGuiHoveredFlags_AnyWindow)))
	{
		return;
	}

	int state = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);

	if (state != GLFW_PRESS)
	{
		return;
	}

	const auto& type = TileTypes[m_selectedTileType];
	size_t atlasTextureIndex = -1;
	bool textureIndexFound = false;

	for (size_t i = 0; i < m_atlasEntries.size() && !textureIndexFound; i++)
	{
		if (m_atlasEntries[i].id == type.textureAtlasEntryId)
		{
			atlasTextureIndex = i;
			textureIndexFound = true;
		}
	}

	if (!textureIndexFound)
	{
		return;
	}

	const auto worldPos = mouseToWorld();

	if (worldPos.x < 0 || worldPos.y < 0)
	{
		return;
	}

	const auto tileRow = static_cast<int16_t>(std::floor(worldPos.y));
	const auto tileColumn = static_cast<int16_t>(std::floor(worldPos.x));

	m_map->setTileAt(
		tileColumn,
		tileRow,
		m_selectedLayer,
		static_cast<size_t>(m_selectedTileType),
		atlasTextureIndex,
		m_selectedFrame);
}

void Editor::drawMap()
{
	const auto& frustum = m_camera->getFrustum();

	const auto& tiles = m_map->getTiles();
	const auto& gameObjects = m_world->getGameObjects();

	const glm::vec3 scale{PIXELS_PER_UNIT, PIXELS_PER_UNIT, 1 };

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
			m_renderer->drawSprite(worldPos, scale, layer.sprite);
		}
	}

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

			m_renderer->drawSprite(worldPosition, scale, sprite);
		}
		else
		{
			m_renderer->drawSprite(worldPosition, scale, gameObject.getSprite());
		}
	}

	if (m_selectedTileType > 0)
	{
		const auto& type = TileTypes[m_selectedTileType];
		size_t atlasTextureIndex = -1;
		bool textureIndexFound = false;

		for (size_t i = 0; i < m_atlasEntries.size() && !textureIndexFound; i++)
		{
			if (m_atlasEntries[i].id == type.textureAtlasEntryId)
			{
				atlasTextureIndex = i;
				textureIndexFound = true;
			}
		}

		if (!textureIndexFound)
		{
			return;
		}

		const auto mouseInWorld = mouseToWorld();
		m_renderer->drawSprite(
			mouseInWorld,
			scale,
			{
				.textureIndex = atlasTextureIndex,
				.currentFrame = m_selectedFrame
			});
	}
}


glm::vec2 Editor::screenToWorld(const glm::vec2& screenPos) const
{
	const auto& frustum = m_camera->getFrustum();
	const auto x = screenPos.x / static_cast<float>(PIXELS_PER_UNIT) + frustum.x;
	const auto y = screenPos.y / static_cast<float>(PIXELS_PER_UNIT) + frustum.y;

	return { x, y };
}

glm::vec3 Editor::mouseToWorld() const
{
	double xpos, ypos;
	glfwGetCursorPos(m_window, &xpos, &ypos);

	return glm::vec3(screenToWorld({xpos, ypos}), 0);
}


void Editor::mouseButtonCallback(int button, int action, int mods)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(button, action);
}

void Editor::handleKeyInput(const Timestep& timestep)
{
	static float secondsSinceLastFrameChange = 0.0f;
	secondsSinceLastFrameChange += timestep.deltaSeconds;

	if (m_selectedTileType > 0 && glfwGetKey(m_window, GLFW_KEY_KP_ADD) == GLFW_PRESS && secondsSinceLastFrameChange > 0.1)
	{
		const auto& type = TileTypes[m_selectedTileType];
		const auto& atlasEntry = m_atlasEntries[type.textureAtlasEntryId - 1];
		m_selectedFrame = static_cast<uint16_t>((m_selectedFrame + 1) % atlasEntry.frames.size());

		secondsSinceLastFrameChange = 0.0f;
	}

    glm::vec3 cameraMovement(0.0f, 0.0f, 0.0f);

    int state = glfwGetKey(m_window, GLFW_KEY_W);
    if (state == GLFW_PRESS)
    {
        cameraMovement.y = -1;
    }

    state = glfwGetKey(m_window, GLFW_KEY_S);
    if (state == GLFW_PRESS)
    {
        cameraMovement.y = 1;
    }

    state = glfwGetKey(m_window, GLFW_KEY_A);
    if (state == GLFW_PRESS)
    {
        cameraMovement.x = -1;
    }

    state = glfwGetKey(m_window, GLFW_KEY_D);
    if (state == GLFW_PRESS)
    {
        cameraMovement.x = 1;
    }

    if (glm::length(cameraMovement) < 0.1f)
    {
        return;
    }

    cameraMovement = glm::normalize(cameraMovement);
    cameraMovement *= 5 * timestep.deltaSeconds;

    m_camera->moveBy(cameraMovement);
}

void Editor::glfwMouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{
    auto game = reinterpret_cast<Editor*>(glfwGetWindowUserPointer(window));
    game->mouseButtonCallback(button, action, mods);
}

void Editor::scrollCallback(double xoffset, double yoffset)
{
	if (yoffset > 0)
	{
		PIXELS_PER_UNIT = std::floor(
			std::max(
				static_cast<float>(PIXELS_PER_UNIT) * ZOOM_STEP_FACTOR,
				static_cast<float>(PIXELS_PER_UNIT + 1)));
	}
	else if (yoffset < 0)
	{
		PIXELS_PER_UNIT = std::floor(static_cast<float>(PIXELS_PER_UNIT) / ZOOM_STEP_FACTOR);
	}

	m_renderer->setPixelsPerUnit(PIXELS_PER_UNIT);

	const auto windowExtent = m_vulkanWindow->getWindowExtent();
	const CameraArea visibleArea
	{
		static_cast<float>(windowExtent.width) / static_cast<float>(PIXELS_PER_UNIT),
		static_cast<float>(windowExtent.height) / static_cast<float>(PIXELS_PER_UNIT),
		1.0f,
		10.0f
	};

	m_camera->setVisibleArea(visibleArea);
}

void Editor::glfwScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
	auto game = reinterpret_cast<Editor*>(glfwGetWindowUserPointer(window));
	game->scrollCallback(xoffset, yoffset);
}

void Editor::handleWindowResize()
{
	const auto windowExtent = m_vulkanWindow->getWindowExtent();

	const CameraArea visibleArea
	{
		static_cast<float>(windowExtent.width) / static_cast<float>(PIXELS_PER_UNIT),
		static_cast<float>(windowExtent.height) / static_cast<float>(PIXELS_PER_UNIT),
		1.0f,
		10.0f
	};

	m_camera = std::make_unique<Camera>(
		glm::vec3(
			static_cast<float>(m_map->getColumns()) / 2.0f,
			static_cast<float>(m_map->getRows()) / 2.0f,
			0.0f),
		visibleArea,
		windowExtent.width,
		windowExtent.height);
}

void Editor::glfwWindowResize(GLFWwindow* window, int width, int height)
{
	auto game = reinterpret_cast<Editor*>(glfwGetWindowUserPointer(window));
	game->handleWindowResize();
}

void Editor::openMap()
{
	char saveFileName[320] = "";

	OPENFILENAME ofn{};
	ofn.lpstrDefExt = ".fecmap";
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = glfwGetWin32Window(m_window);
	ofn.lpstrFilter = "FEC-Map files (*.fecmap)\0*.fecmap\0";
	ofn.lpstrFile = saveFileName;
	ofn.nMaxFile = 320;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn))
	{
		const auto result = MapSerializer::deserializeMap(saveFileName);
		m_map = std::make_unique<Map>(result.map);
		m_layerCount = result.layers;
	}
}


void Editor::saveMap()
{
	char saveFileName[320] = "";

	OPENFILENAME ofn{};
	ofn.lpstrDefExt = ".fecmap";
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = glfwGetWin32Window(m_window);
	ofn.lpstrFilter = "FEC-Map files (*.fecmap)\0*.fecmap\0";
	ofn.lpstrFile = saveFileName;
	ofn.nMaxFile = 320;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		MapSerializer::serializeMap(saveFileName, *m_map);
	}
}

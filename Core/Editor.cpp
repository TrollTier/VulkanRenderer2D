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

	while (!glfwWindowShouldClose(m_window))
    {
        const auto startOfFrame = std::chrono::high_resolution_clock::now();

        glfwPollEvents();

        auto startOfCurrentUpdate = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float, std::milli> durationSinceLastUpdate =
            startOfCurrentUpdate - startOfLastUpdate;

        const Timestep step
        {
            durationSinceLastUpdate.count(),
            durationSinceLastUpdate.count() / 1000
        };

        handleKeyInput(step);
    	updateAnimations(step);
		setSelectedTile();

        startOfLastUpdate = startOfCurrentUpdate;

        const auto startOfRender = std::chrono::high_resolution_clock::now();

		updateUI();
    	ImDrawData* uiData = ImGui::GetDrawData();
    	m_renderer->draw_scene(*m_camera, *m_map, *m_world, m_atlasEntries, uiData);

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
	static float timeSinceLastUpdateLoop = 0;

	if (!m_runAnimations)
	{
		return;
	}

	timeSinceLastUpdateLoop += step.deltaMilliseconds;

	if (timeSinceLastUpdateLoop >= 100)
	{
		for (auto& tile : m_map->getTiles())
		{
			const auto& atlasEntry = m_atlasEntries[tile.sprite.textureIndex];
			tile.sprite.currentFrame = (tile.sprite.currentFrame + 1) % atlasEntry.frames.size();
		}

		timeSinceLastUpdateLoop = 0;
	}
}

void Editor::updateUI()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Stats", &m_showImGui, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("This is some useful text.");

	if (ImGui::Button("Save"))
	{
		std::cout << "Saving..." << std::endl;
	}

	if (ImGui::Button("Animations"))
	{
		m_runAnimations = !m_runAnimations;
	}

	for (size_t i = 0; i < TileTypes.size(); i++)
	{
		if (ImGui::Button(TileTypes[i].tileName.data()))
		{
			m_selectedTileType = static_cast<int32_t>(i);
		}
	}

	ImGui::End();
	ImGui::Render();
}

void Editor::setSelectedTile()
{
	if (m_selectedTileType < 0 || ImGui::IsWindowHovered((ImGuiHoveredFlags_AnyWindow)))
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

	double xpos, ypos;
	glfwGetCursorPos(m_window, &xpos, &ypos);
	const auto worldPos = screenToWorld({xpos, ypos});

	if (worldPos.x < 0 || worldPos.y < 0)
	{
		return;
	}

	const auto tileRow = static_cast<int16_t>(std::floor(worldPos.y));
	const auto tileColumn = static_cast<int16_t>(std::floor(worldPos.x));

	if (m_map->isInMap(tileColumn, tileRow))
	{
		auto& tile = m_map->getTileAt(tileColumn, tileRow);
		tile.tileDataIndex = static_cast<size_t>(m_selectedTileType);
		tile.sprite.textureIndex = atlasTextureIndex;
		tile.sprite.currentFrame = type.frameIndex;
	}
}

glm::vec2 Editor::screenToWorld(const glm::vec2& screenPos) const
{
	const auto& frustum = m_camera->getFrustum();
	const auto x = screenPos.x / static_cast<float>(PIXELS_PER_UNIT) + frustum.x;
	const auto y = screenPos.y / static_cast<float>(PIXELS_PER_UNIT) + frustum.y;

	return { x, y };
}

void Editor::mouseButtonCallback(int button, int action, int mods)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddMouseButtonEvent(button, action);
}

void Editor::handleKeyInput(const Timestep& timestep)
{
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
		glm::vec3((float)m_map->getColumns() / 2.0f, (float)m_map->getRows() / 2.0f, 0.0f),
		visibleArea,
		windowExtent.width,
		windowExtent.height);
}


void Editor::glfwWindowResize(GLFWwindow* window, int width, int height)
{
	auto game = reinterpret_cast<Editor*>(glfwGetWindowUserPointer(window));
	game->handleWindowResize();
}



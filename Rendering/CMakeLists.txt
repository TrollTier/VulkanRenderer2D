add_library(Rendering
    VulkanRenderer.cpp
        VulkanHelpers.h
        Vertex.h
        Sampler.h
        Descriptors.h
        Instance.h
        Swapchain.h
        Swapchain.cpp
        SwapchainElement.h
        VulkanRessources.cpp
        VulkanRessources.h
        VulkanWindow.cpp
        VulkanWindow.h
)

target_link_libraries(Rendering PRIVATE Vulkan::Vulkan)
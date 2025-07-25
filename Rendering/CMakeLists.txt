add_library(Rendering
    VulkanRenderer.cpp
        VulkanHelpers.h
        ../Core/Vertex.h
        Sampler.h
        Instance.h
        Swapchain.h
        Swapchain.cpp
        SwapchainElement.h
        VulkanRessources.cpp
        VulkanRessources.h
        VulkanWindow.cpp
        VulkanWindow.h
        Pipeline.cpp
        Pipeline.h
        UniformBufferObject.h
        Texture2D.cpp
        Texture2D.h
        ImageLoader.h
        InstanceData.h
)

target_link_libraries(Rendering PRIVATE Vulkan::Vulkan)
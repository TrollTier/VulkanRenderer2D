add_library(Rendering
    VulkanRenderer.cpp
        VulkanHelpers.h
        ../Core/Vertex.h
        Swapchain.h
        Swapchain.cpp
        SwapchainElement.h
        VulkanRessources.cpp
        VulkanRessources.h
        VulkanWindow.cpp
        VulkanWindow.h
        Pipeline.cpp
        Pipeline.h
        Texture2D.cpp
        Texture2D.h
        ImageLoader.h
        InstanceData.h
        CameraUniformData.h
        Buffer.cpp
        Buffer.h
)

target_link_libraries(Rendering PRIVATE Vulkan::Vulkan glfw)
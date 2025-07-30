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
        ObjectPushConstants.h
        Texture2D.cpp
        Texture2D.h
        ImageLoader.h
        InstanceData.h
        CameraUniformData.h
        Buffer.cpp
        Buffer.h
)

target_include_directories(Rendering PUBLIC ../include/glfw-3.4/include)
target_link_libraries(Rendering PRIVATE Vulkan::Vulkan glfw)

set(Pinut_core
    src/core/application.cpp
    src/core/application.h
    src/core/camera.cpp
    src/core/camera.h
    src/core/defines.h
    src/core/light.h
    src/core/scene.cpp
    src/core/scene.h)

set(Pinut_imgui
    src/imgui/pinutImgui.cpp
    src/imgui/pinutImgui.h)

set(Pinut_renderer
    src/renderer/buffer.cpp
    src/renderer/buffer.h
    src/renderer/commandBufferManager.cpp
    src/renderer/commandBufferManager.h
    src/renderer/common.h
    src/renderer/descriptorSetManager.cpp
    src/renderer/descriptorSetManager.h
    src/renderer/device.cpp
    src/renderer/device.h
    src/renderer/material.cpp
    src/renderer/material.h
    src/renderer/mesh.cpp
    src/renderer/mesh.h
    src/renderer/pipeline.cpp
    src/renderer/pipeline.h
    src/renderer/primitives.cpp
    src/renderer/primitives.h
    src/renderer/renderable.cpp
    src/renderer/renderable.h
    src/renderer/swapchain.cpp
    src/renderer/swapchain.h
    src/renderer/texture.cpp
    src/renderer/texture.h
    src/renderer/utils.cpp
    src/renderer/utils.h)

set(Pinut_pipelines
    src/renderer/pipelines/forward.cpp
    src/renderer/pipelines/forward.h)

add_library(Pinut STATIC ${Pinut_core} ${Pinut_imgui} ${Pinut_renderer} ${Pinut_pipelines})

add_subdirectory(shaders)
add_subdirectory(external/glfw)
add_subdirectory(external/imgui)
add_subdirectory(external/vkbootstrap)
add_subdirectory(external/vma)

# add_subdirectory(external/glm)
# add_subdirectory(external/stb)
target_link_libraries(Pinut PUBLIC GLFW IMGUI VKBOOTSTRAP VMA ${Vulkan_LIBRARY})

# target_link_libraries(Pinut PUBLIC GLFW GLM STB VMA VKBOOTSTRAP ${Vulkan_LIBRARY})
target_include_directories(Pinut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIR})
target_compile_features(Pinut PRIVATE cxx_std_20)
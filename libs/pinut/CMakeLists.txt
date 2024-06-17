
set(Pinut_core
    src/core/application.cpp
    src/core/application.h
    src/core/buffer.cpp
    src/core/buffer.h
    src/core/common.cpp
    src/core/common.h
    src/core/commandBufferRing.cpp
    src/core/commandBufferRing.h
    src/core/descriptorSetManager.cpp
    src/core/descriptorSetManager.h
    src/core/device.cpp
    src/core/device.h
    src/core/gbuffer.cpp
    src/core/gbuffer.h
    src/core/swapchain.cpp
    src/core/swapchain.h
    src/core/texture.cpp
    src/core/texture.h
    src/core/vulkan_utils.cpp
    src/core/vulkan_utils.h)

set(Pinut_misc
    src/misc/camera.cpp
    src/misc/camera.h)

set(Pinut_pipelines
    src/pipelines/forward.cpp
    src/pipelines/forward.h
    src/pipelines/pipelineBuilder.cpp
    src/pipelines/pipelineBuilder.h)

set(Pinut_scene
    src/scene/material.cpp
    src/scene/material.h
    src/scene/mesh.cpp
    src/scene/mesh.h
    src/scene/scene.cpp
    src/scene/scene.h)

add_library(Pinut STATIC ${Pinut_core} ${Pinut_pipelines} ${Pinut_scene} ${Pinut_misc})

add_subdirectory(libs/glfw)
add_subdirectory(libs/glm)
add_subdirectory(libs/stb)
add_subdirectory(libs/vkbootstrap)
add_subdirectory(libs/vma)

target_link_libraries(Pinut PUBLIC GLFW GLM STB VMA VKBOOTSTRAP ${Vulkan_LIBRARY})
target_include_directories(Pinut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIR})
target_compile_features(Pinut PRIVATE cxx_std_20)

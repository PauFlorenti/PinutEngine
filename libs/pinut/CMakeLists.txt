
set(Pinut_assets
    src/assets/mesh.cpp
    src/assets/mesh.h
    src/assets/material.cpp
    src/assets/material.h
    src/assets/texture.cpp
    src/assets/texture.h)

set(Pinut_core
    src/core/application.cpp
    src/core/application.h
    src/core/assetManager.cpp
    src/core/assetManager.h
    src/core/camera.cpp
    src/core/camera.h
    src/core/defines.h
    src/core/entity.cpp
    src/core/entity.h
    src/core/light.cpp
    src/core/light.h
    src/core/node.cpp
    src/core/node.h
    src/core/renderable.cpp
    src/core/renderable.h
    src/core/scene.cpp
    src/core/scene.h)

set(Pinut_loaders
    src/loaders/gltfLoader.cpp
    src/loaders/gltfLoader.h
    src/loaders/objLoader.cpp
    src/loaders/objLoader.h)

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
    src/renderer/pipeline.cpp
    src/renderer/pipeline.h
    src/renderer/primitives.cpp
    src/renderer/primitives.h
    src/renderer/swapchain.cpp
    src/renderer/swapchain.h
    src/renderer/utils.cpp
    src/renderer/utils.h)

set(Pinut_pipelines
    src/renderer/pipelines/forward.cpp
    src/renderer/pipelines/forward.h)

set(Pinut_stages
    src/renderer/stages/materialManager.cpp
    src/renderer/stages/materialManager.h
    src/renderer/stages/opaqueStage.cpp
    src/renderer/stages/opaqueStage.h
    src/renderer/stages/pipelineStage.cpp
    src/renderer/stages/pipelineStage.h
    src/renderer/stages/skyboxStage.cpp
    src/renderer/stages/skyboxStage.h
    src/renderer/stages/transparentStage.cpp
    src/renderer/stages/transparentStage.h
    src/renderer/stages/wireframeStage.cpp
    src/renderer/stages/wireframeStage.h)

add_library(Pinut STATIC ${Pinut_assets} ${Pinut_core} ${Pinut_loaders} ${Pinut_imgui} ${Pinut_renderer} ${Pinut_pipelines} ${Pinut_stages})

# tinygltf
add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf SYSTEM INTERFACE ${TINYGLTF_DIR})

add_subdirectory(shaders)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/imgui)

# add_subdirectory(external/tinygltf)
add_subdirectory(external/tinyobjloader)
add_subdirectory(external/vkbootstrap)
add_subdirectory(external/vma)

# add_subdirectory(external/stb)
target_link_libraries(Pinut PUBLIC GLFW glm IMGUI tinygltf TINYOBJ VKBOOTSTRAP VMA ${Vulkan_LIBRARY})
target_include_directories(Pinut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIR})
target_compile_features(Pinut PRIVATE cxx_std_20)
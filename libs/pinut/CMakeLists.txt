
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
    src/renderer/common.h
    src/renderer/primitives.cpp
    src/renderer/primitives.h
    src/renderer/renderer.cpp
    src/renderer/renderer.h)

set(Pinut_pipelines
    src/renderer/pipelines/forward.cpp
    src/renderer/pipelines/forward.h)

# files_glob_recurse(pinut_source_files "src/*.cpp src/*.h")
add_library(Pinut STATIC ${Pinut_assets} ${Pinut_core} ${Pinut_loaders} ${Pinut_imgui} ${Pinut_renderer} ${Pinut_pipelines})

# glm
# add_library(glm INTERFACE)
# files_glob_recurse(glm_source_files "external/glm/glm/*.cpp" "external/glm/glm/*.h")
# MESSAGE("GLM Files " ${glm_source_files})
# target_sources(glm INTERFACE ${glm_source_files})

# add_subdirectory(external/glm)
# add_subdirectory(external/imgui)

# imgui
add_library(imgui INTERFACE)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
target_sources(imgui INTERFACE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    # ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp

    # ${IMGUI_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp

    # ${IMGUI_DIR}/backends/imgui_impl_vulkan.h
)

# Set the include directories for ImGui
target_include_directories(imgui INTERFACE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ImGuizmo
add_library(imguizmo INTERFACE)
set(IMGUIZMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ImGuizmo)
target_sources(imguizmo INTERFACE ${IMGUIZMO_DIR}/ImGuizmo.h ${IMGUIZMO_DIR}/ImGuizmo.cpp)
target_include_directories(imguizmo SYSTEM INTERFACE ${IMGUIZMO_DIR} ${IMGUI_DIR})

# stb
add_library(stb INTERFACE)
set(STB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)
target_sources(stb INTERFACE ${STB_DIR}/stb_image.h ${STB_DIR}/stb_image_write.h)

# tinygltf
add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf SYSTEM INTERFACE ${TINYGLTF_DIR})

# tinyobjloader
add_library(tinyobj INTERFACE)
set(TINYOBJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader)
target_sources(tinyobj INTERFACE ${TINYOBJ_DIR}/tiny_obj_loader.cc ${TINYGLTF_DIR}/tiny_obj_loader.h)

# VKBOOTSTRAP
add_library(VKBOOTSTRAP INTERFACE)
files_glob_recurse(vkbootstrap_sources "external/vk-bootstrap/src/*.cpp" "external/vk-bootstrap/src/*.h")
set(VKBOOTSTRAP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/vk-bootstrap/include)
target_sources(VKBOOTSTRAP INTERFACE ${vkbootstrap_sources})

add_subdirectory(shaders)
add_subdirectory(external/glfw)

target_link_libraries(Pinut PUBLIC PRIVATE glfw RenderDevice imgui imguizmo VKBOOTSTRAP)
target_include_directories(Pinut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${GLFW_SOURCE_DIR}/include ${Vulkan_INCLUDE_DIR})
target_compile_features(Pinut PRIVATE cxx_std_20)
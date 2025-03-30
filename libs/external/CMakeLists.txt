
# Entt
add_library(entt INTERFACE)
set(ENTT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/entt/single_include)
target_sources(entt INTERFACE ${ENTT_DIR}/entt/entt.hpp)
target_include_directories(entt SYSTEM INTERFACE ${ENTT_DIR})

# glm
add_library(glm INTERFACE)
set(glm_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm)
files_glob_recurse(glm_source_files "glm/glm/*.cpp" "glm/glm/*.h")
target_sources(glm INTERFACE ${glm_source_files})
target_include_directories(glm SYSTEM INTERFACE ${glm_DIR})

# imgui
add_library(imgui INTERFACE)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
target_sources(imgui INTERFACE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

# Set the include directories for ImGui
target_include_directories(imgui SYSTEM INTERFACE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# ImGuizmo
add_library(imguizmo INTERFACE)
set(IMGUIZMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo)
target_sources(imguizmo INTERFACE ${IMGUIZMO_DIR}/ImGuizmo.h ${IMGUIZMO_DIR}/ImGuizmo.cpp)
target_include_directories(imguizmo SYSTEM INTERFACE ${IMGUIZMO_DIR} ${IMGUI_DIR})

# stb
add_library(stb INTERFACE)
set(STB_DIR ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(stb INTERFACE ${STB_DIR}/stb/stb_image.h ${STB_DIR}/stb/stb_image_write.h)
target_include_directories(stb SYSTEM INTERFACE ${STB_DIR})

# tinygltf
add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf SYSTEM INTERFACE ${TINYGLTF_DIR})

# tinyobjloader
add_library(tinyobj INTERFACE)
set(TINYOBJ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader)
target_sources(tinyobj INTERFACE ${TINYOBJ_DIR}/tiny_obj_loader.cc ${TINYOBJ_DIR}/tiny_obj_loader.h)

# VKBOOTSTRAP
add_library(VKBOOTSTRAP INTERFACE)
files_glob_recurse(vkbootstrap_sources "vk-bootstrap/src/*.cpp" "vk-bootstrap/src/*.h")
set(VKBOOTSTRAP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vk-bootstrap/src)
target_sources(VKBOOTSTRAP INTERFACE ${vkbootstrap_sources})
target_include_directories(VKBOOTSTRAP SYSTEM INTERFACE ${VKBOOTSTRAP_DIR})

add_subdirectory(glfw)
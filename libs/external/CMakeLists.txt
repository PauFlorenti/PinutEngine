# Entt
add_library(entt INTERFACE)
set(ENTT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/entt/single_include)
target_sources(entt INTERFACE ${ENTT_DIR}/entt/entt.hpp)
target_include_directories(entt SYSTEM INTERFACE ${ENTT_DIR})

# glm
files_glob_recurse(glm_source_files "glm/glm/*.cpp" "glm/glm/*.h")
set(glm_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm)
add_library(glm STATIC ${glm_source_files})
target_include_directories(glm PUBLIC ${glm_DIR})

# imgui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

# Set the include directories for ImGui
target_include_directories(imgui
    PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    PRIVATE
    ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(imgui PRIVATE glfw ${Vulkan_LIBRARIES})

# ImGuizmo
set(IMGUIZMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo)
add_library(imguizmo STATIC ${IMGUIZMO_DIR}/ImGuizmo.h ${IMGUIZMO_DIR}/ImGuizmo.cpp)
target_include_directories(imguizmo PUBLIC ${IMGUIZMO_DIR} PRIVATE ${IMGUI_DIR})
target_link_libraries(imguizmo PRIVATE imgui)

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
files_glob_recurse(vkbootstrap_sources "vk-bootstrap/src/*.cpp" "vk-bootstrap/src/*.h")
set(VKBOOTSTRAP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vk-bootstrap/src)
add_library(VKBOOTSTRAP STATIC ${vkbootstrap_sources})
target_include_directories(VKBOOTSTRAP PUBLIC ${VKBOOTSTRAP_DIR} PRIVATE ${Vulkan_INCLUDE_DIR})

# VMA
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include/)
add_library(VMA INTERFACE)
target_sources(VMA INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(VMA SYSTEM INTERFACE ${VMA_DIR})

add_subdirectory(glfw)
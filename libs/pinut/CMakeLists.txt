files_glob_recurse(source_files "src/*.cpp" "src/*.h" "src/pch.hpp")
add_library(Pinut STATIC ${source_files})

# glm
# add_library(glm INTERFACE)
# files_glob_recurse(glm_source_files "external/glm/glm/*.cpp" "external/glm/glm/*.h")
# MESSAGE("GLM Files " ${glm_source_files})
# target_sources(glm INTERFACE ${glm_source_files})

# add_subdirectory(external/glm)

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
target_sources(tinyobj INTERFACE ${TINYOBJ_DIR}/tiny_obj_loader.cc ${TINYOBJ_DIR}/tiny_obj_loader.h)

# VKBOOTSTRAP
add_library(VKBOOTSTRAP INTERFACE)
files_glob_recurse(vkbootstrap_sources "external/vk-bootstrap/src/*.cpp" "external/vk-bootstrap/src/*.h")
set(VKBOOTSTRAP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/vk-bootstrap/include)
target_sources(VKBOOTSTRAP INTERFACE ${vkbootstrap_sources})

add_subdirectory(shaders)
add_subdirectory(external/glfw)

target_link_libraries(Pinut PUBLIC RenderDevice PRIVATE glfw imgui imguizmo tinyobj VKBOOTSTRAP)
target_include_directories(Pinut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${GLFW_SOURCE_DIR}/include ${Vulkan_INCLUDE_DIR})
target_compile_features(Pinut PRIVATE cxx_std_20)
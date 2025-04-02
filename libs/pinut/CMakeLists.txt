files_glob_recurse(source_files "src/*.cpp" "include/*.h")

# Pinut static library
add_library(Pinut STATIC ${source_files})
set_target_properties(RenderDevice
    PROPERTIES LINKER_LANGUAGE CXX
    COMPILE_WARNING_AS_ERROR ON)

target_compile_features(Pinut
    PRIVATE cxx_std_20)

target_precompile_headers(Pinut PRIVATE pch.hpp)

target_link_libraries(Pinut
    PUBLIC common
    RenderDevice
    entt
    glfw
    VKBOOTSTRAP
    PRIVATE common
    glm
    stb
    imgui
    imguizmo
    tinygltf
    tinyobj
    ${Vulkan_LIBRARIES})

target_include_directories(Pinut
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLFW_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR})

add_subdirectory(shaders)

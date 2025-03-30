files_glob_recurse(source_files "src/*.cpp" "include/*.h" "include/pch.hpp")

# Pinut static library
add_library(Pinut STATIC ${source_files})
target_compile_features(Pinut PRIVATE cxx_std_20)
target_precompile_headers(Pinut PRIVATE include/pch.hpp)
target_link_libraries(Pinut
    PUBLIC
    RenderDevice entt glfw VKBOOTSTRAP
    PRIVATE
    glm stb imgui imguizmo tinygltf tinyobj ${Vulkan_LIBRARIES})
target_include_directories(Pinut
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${GLFW_SOURCE_DIR}/include ${Vulkan_INCLUDE_DIR})

add_subdirectory(shaders)

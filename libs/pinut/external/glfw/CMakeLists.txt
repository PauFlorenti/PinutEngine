add_library(GLFW INTERFACE)
target_link_libraries(GLFW INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib)
target_include_directories(GLFW INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

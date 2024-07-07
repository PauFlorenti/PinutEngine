set(IMGUI_SRC
    imconfig.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp
    imgui.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_glfw.h
    backends/imgui_impl_vulkan.cpp
    backends/imgui_impl_vulkan.h)

add_library(IMGUI STATIC ${IMGUI_SRC})

target_include_directories(IMGUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${Vulkan_INCLUDE_DIR})
target_compile_features(IMGUI PRIVATE cxx_std_20)

set(VKBOOTSTRAP_SRC
    VkBootstrap.cpp
    VkBootstrap.h
    VkBootstrapDispatch.h)

add_library(VKBOOTSTRAP STATIC ${VKBOOTSTRAP_SRC})

target_include_directories(VKBOOTSTRAP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIR})
target_compile_features(VKBOOTSTRAP PRIVATE cxx_std_20)

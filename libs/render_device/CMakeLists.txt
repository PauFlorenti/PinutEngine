files_glob_recurse(source_files "src/*.cpp" "include/*.h" "include/pch.hpp")

# RenderDevice library
add_library(RenderDevice ${source_files})
set_target_properties(RenderDevice PROPERTIES LINKER_LANGUAGE CXX)
target_precompile_headers(RenderDevice PRIVATE include/pch.hpp)
target_include_directories(RenderDevice PUBLIC include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/internal ${CMAKE_CURRENT_SOURCE_DIR}/external ${Vulkan_INCLUDE_DIR})
target_link_libraries(RenderDevice PRIVATE VMA ${Vulkan_LIBRARY})
target_compile_features(RenderDevice PRIVATE cxx_std_20)

# VMA
add_library(VMA INTERFACE)
Set(VMA_DIR ${CMAKE_SOURCE_DIR}/libs/external/VulkanMemoryAllocator/include/)
target_sources(VMA INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(VMA SYSTEM INTERFACE ${VMA_DIR})

files_glob_recurse(source_files "src/*.cpp" "src/*.h" "src/pch.hpp")

# RenderDevice library
add_library(RenderDevice ${source_files})
set_target_properties(RenderDevice PROPERTIES LINKER_LANGUAGE CXX)
target_precompile_headers(RenderDevice PRIVATE src/pch.hpp)
target_include_directories(RenderDevice PRIVATE . ${Vulkan_INCLUDE_DIR})
target_link_libraries(RenderDevice PUBLIC VMA ${Vulkan_LIBRARY})
target_compile_features(RenderDevice PRIVATE cxx_std_20)

# VMA
add_library(VMA INTERFACE)
target_sources(VMA INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/external/VulkanMemoryAllocator/include/vk_mem_alloc.h)

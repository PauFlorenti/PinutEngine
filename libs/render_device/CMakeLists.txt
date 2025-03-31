files_glob_recurse(source_files "src/*.cpp" "include/*.h")

# RenderDevice library
add_library(RenderDevice ${source_files})
set_target_properties(RenderDevice PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(RenderDevice PRIVATE cxx_std_20)
target_precompile_headers(RenderDevice PRIVATE pch.hpp)
target_include_directories(RenderDevice 
PUBLIC 
    include 
PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/internal ${Vulkan_INCLUDE_DIR})

target_link_libraries(RenderDevice 
PRIVATE 
    common VMA ${Vulkan_LIBRARY})

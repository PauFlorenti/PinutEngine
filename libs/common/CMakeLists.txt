files_glob_recurse(source_files "src/*.cpp" "include/*.h")

# Common library
add_library(common ${source_files})
set_target_properties(common
    PROPERTIES
    LINKER_LANGUAGE CXX
    COMPILE_WARNING_AS_ERROR ON)

target_compile_features(common
    PRIVATE
    cxx_std_20)

target_precompile_headers(common
    PRIVATE
    pch.hpp)

target_include_directories(common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

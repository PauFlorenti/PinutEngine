set(TINYOBJ_SRC
    tiny_obj_loader.cc
    tiny_obj_loader.h)

add_library(TINYOBJ STATIC ${TINYOBJ_SRC})

target_include_directories(TINYOBJ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(TINYOBJ PRIVATE cxx_std_20)

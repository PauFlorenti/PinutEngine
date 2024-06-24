set(src_files
    sandbox.cpp
    sandbox.h
    stdafx.cpp
    stdafx.h
)

add_executable(Sandbox ${src_files})

target_link_libraries(Sandbox LINK_PUBLIC Pinut ${Vulkan_LIBRARIES})
target_compile_features(Sandbox PRIVATE cxx_std_20)
set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_HOME_DIRECTORY}/bin" DEBUG_POSTFIX "d")

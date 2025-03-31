add_executable(Sandbox src/sandbox.cpp src/sandbox.h)

set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_HOME_DIRECTORY}/bin" DEBUG_POSTFIX "d")
target_compile_features(Sandbox PRIVATE cxx_std_20)
target_precompile_headers(Sandbox PRIVATE pch.hpp)
target_link_libraries(Sandbox PRIVATE Pinut ${Vulkan_LIBRARIES})
target_include_directories(Sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIR})

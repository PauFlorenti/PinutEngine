add_executable(Sandbox src/sandbox.cpp src/sandbox.h src/pch.hpp src/pch.cpp)

target_link_libraries(Sandbox PUBLIC Pinut ${Vulkan_LIBRARIES})
target_compile_features(Sandbox PRIVATE cxx_std_20)
target_precompile_headers(Sandbox PRIVATE src/pch.hpp)
set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_HOME_DIRECTORY}/bin" DEBUG_POSTFIX "d")

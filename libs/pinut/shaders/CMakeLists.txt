file(GLOB_RECURSE shader_files CONFIGURE_DEPENDS *.frag *.vert)

set(glslc "D:/TOOLS/Vulkan/1.3.231.1/Bin/glslc.exe")
set(objects)

foreach(file ${shader_files})
    cmake_path(GET file STEM stem)

    if(${file} MATCHES ".*.vert")
        set(file_name "${stem}.vert")
    elseif(${file} MATCHES ".*.frag")
        set(file_name "${stem}.frag")
    endif()

    set(output "${CMAKE_HOME_DIRECTORY}/bin/shaders/${file_name}.spv")

    add_custom_command(
        OUTPUT "${output}"
        MAIN_DEPENDENCY ${file}
        COMMENT "Compiling Vulkan shader '${file}'"
        COMMAND "${glslc}" "${file}" -o "${output}"
    )
    list(APPEND objects "${output}")
endforeach()

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX "Source files"
    FILES ${shader_files})

install(
    FILES ${objects}
    DESTINATION bin/shaders
    COMPONENT Pinut
)

add_custom_target(Shaders DEPENDS ${objects})
add_dependencies(Pinut Shaders)

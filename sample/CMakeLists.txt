
add_executable(00-triangle 00-triangle/main.cpp)

set_target_properties(00-triangle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${bin_dir}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${bin_dir}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${bin_dir}
)

target_include_directories(00-triangle PRIVATE ${inc_dir})
target_link_directories(00-triangle PRIVATE ${lib_dir})
target_link_libraries(00-triangle PRIVATE engine)



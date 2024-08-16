
# add_executable(00-triangle 
#     00-triangle/main.cpp
#     00-triangle/Triangle.h 
#     00-triangle/Triangle.cpp)

# set_target_properties(00-triangle PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${bin_dir}
#     RUNTIME_OUTPUT_DIRECTORY_DEBUG ${bin_dir}
#     RUNTIME_OUTPUT_DIRECTORY_RELEASE ${bin_dir}
# )

# target_include_directories(00-triangle PRIVATE ${inc_dir})
# target_link_directories(00-triangle PRIVATE ${lib_dir})
# target_link_libraries(00-triangle PRIVATE engine)


set(samples
    00-triangle
    01-texture
)

foreach(sample  ${samples})
    set(sample_dir ${CMAKE_CURRENT_SOURCE_DIR}/${sample})
    file(GLOB_RECURSE srcs ${sample_dir}/*.h ${sample_dir}/*.cpp)
    add_executable(${sample} ${srcs})

    set_target_properties(${sample} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${bin_dir}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${bin_dir}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${bin_dir})
        
    target_include_directories(${sample} PRIVATE ${inc_dir})
    target_link_directories(${sample} PRIVATE ${lib_dir})
    target_link_libraries(${sample} PRIVATE engine)

endforeach()



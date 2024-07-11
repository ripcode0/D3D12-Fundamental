
file(GLOB_RECURSE srcs *.h *.cpp)

add_library(engine STATIC ${srcs})

set_target_properties(engine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${lib_dir}
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${lib_dir}
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${lib_dir}
)

target_include_directories(engine
    PUBLIC ${inc_dor} ${inc_dir}/extension
)


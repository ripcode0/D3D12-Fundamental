cmake_minimum_required(VERSION 3.22.0)

project(d3d12-fundamental)

set(bin_dir ${CMAKE_SOURCE_DIR}/bin)
set(lib_dir ${CMAKE_SOURCE_DIR}/lib)
set(inc_dir ${CMAKE_SOURCE_DIR}/d3d12)

add_subdirectory(d3d12)


add_subdirectory(sample)
cmake_minimum_required(VERSION 3.10)
project(CUSUM C)

add_library(cusum STATIC cusum_bench.c)
target_link_libraries(cusum PRIVATE m)
target_include_directories(cusum PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

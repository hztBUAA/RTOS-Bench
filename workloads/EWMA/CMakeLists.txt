cmake_minimum_required(VERSION 3.10)
project(EWMA C)

add_library(ewma STATIC ewma_bench.c)
target_link_libraries(ewma PRIVATE m)
target_include_directories(ewma PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

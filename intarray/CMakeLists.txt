cmake_minimum_required(VERSION 2.6)

project(intarray)

set(SRCS wat_array.cpp count2d.cpp)
set(HEADERS wat_array.h count2d.h)

add_library(intarray ${SRCS} ${HEADERS})
target_link_libraries(intarray bitarray sdarray)

add_executable(t_wat gridtest.cpp)
target_link_libraries(t_wat intarray)
set_target_properties(t_wat PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(t_count2d count2d_test.cpp)
target_link_libraries(t_count2d intarray mem utils)
set_target_properties(t_count2d PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
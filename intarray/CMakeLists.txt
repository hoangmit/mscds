cmake_minimum_required(VERSION 2.6)

project(intarray)

set(SRCS wat_array.cpp)
set(HEADERS wat_array.h)

add_library(intarray ${SRCS} ${HEADERS})
target_link_libraries(intarray bitarray)

add_executable(wattest gridtest.cpp)
target_link_libraries(wattest intarray)
set_target_properties(wattest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
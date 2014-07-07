cmake_minimum_required(VERSION 2.6)

project(wavarray)

set(SRCS count2d.cpp)
set(HEADERS wat_array.h count2d.h wat_array.hxx)

add_library(wavarray ${SRCS} ${HEADERS})
target_link_libraries(wavarray bitarray intarray)
#add_sources(mscdsa ${SRCS} ${HEADERS})

add_test_files(gridtest.cpp)

add_test_files(count2d_test.cpp)

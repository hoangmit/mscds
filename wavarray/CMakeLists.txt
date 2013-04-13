cmake_minimum_required(VERSION 2.6)

project(wavarray)

set(SRCS wat_array.cpp count2d.cpp)
set(HEADERS wat_array.h count2d.h)

add_library(wavarray ${SRCS} ${HEADERS})
target_link_libraries(wavarray bitarray intarray)
#add_sources(mscdsa ${SRCS} ${HEADERS})

add_test_files(gridtest.cpp)
#add_executable(t_wat gridtest.cpp)
#target_link_libraries(t_wat wavarray utils)
#set_target_properties(t_wat PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_test_files(count2d_test.cpp)
#add_executable(t_count2d count2d_test.cpp)
#target_link_libraries(t_count2d wavarray ${COMMON_LIBS})
#set_target_properties(t_count2d PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
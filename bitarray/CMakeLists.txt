cmake_minimum_required(VERSION 2.6)

project(bitarray)

set(SRCS rank6p.cpp bitop.cpp bitarray.cpp)
set(HEADERS bitarray.h rank6p.h rank25p.h rankselect.h bitop.h)

#add_library(bitarray ${SRCS} ${HEADERS})
#target_link_libraries(bitarray)
add_sources(mscdsa ${SRCS})

add_executable(t_rank ranktest.cpp)
target_link_libraries(t_rank mscdsa)
set_target_properties(t_rank PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})


add_executable(t_bit bit_test.cpp)
target_link_libraries(t_bit mscdsa)
set_target_properties(t_bit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
cmake_minimum_required(VERSION 2.6)

project(bitarray)

set(SRCS rank6p.cpp rank3p.cpp bitop.cpp bitarray.cpp rrr.cpp rrr2.cpp)
set(HEADERS bitarray.h rank6p.h rank3p.h rank25p.h rankselect.h bitop.h bitstream.h rrr.h rrr2.h)

add_library(bitarray ${SRCS} ${HEADERS})
target_link_libraries(bitarray)
#add_sources(mscdsa ${SRCS} ${HEADERS})

add_test_files(ranktest.cpp)
#add_executable(t_rank ranktest.cpp)
#target_link_libraries(t_rank bitarray utils)
#set_target_properties(t_rank PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
#set_property(TARGET t_rank PROPERTY FOLDER "Tests")


add_test_files(bit_test.cpp bit_benchmark.cpp)
#add_executable(t_bit bit_test.cpp)
#target_link_libraries(t_bit bitarray utils)
#set_target_properties(t_bit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
#set_property(TARGET t_bit PROPERTY FOLDER "Tests")
cmake_minimum_required(VERSION 2.6)

project(bitarray)

set(SRCS rank6p.cpp sdarray.cpp)
set(HEADERS bitarray.h rank6p.h rank25p.h rankselect.h bitop.h sdarray.h)

add_library(bitarray ${SRCS} ${HEADERS})

add_executable(ranktest ranktest.cpp)
target_link_libraries(ranktest bitarray)
set_target_properties(ranktest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})


add_executable(sda_test sda_test.cpp)
target_link_libraries(sda_test bitarray mem)
set_target_properties(sda_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

cmake_minimum_required(VERSION 2.6)

project(bitarray)

set(SRCS rank6p.cpp)
set(HEADERS bitarray.h rank6p.h rank25p.h rankselect.h bitop.h)

add_library(bitarray ${SRCS} ${HEADERS})

add_executable(ranktest ranktest.cpp)
target_link_libraries(ranktest bitarray)
set_target_properties(ranktest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

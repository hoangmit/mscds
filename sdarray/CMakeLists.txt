cmake_minimum_required(VERSION 2.6)

project(sdarray)

set(SRCS sdarray.cpp RLSum.cpp sdarray_small.cpp)
set(HEADERS sdarray.h rlsum_trivbin.h RLSum.h sdarray_small.h)

#add_library(sdarray ${SRCS} ${HEADERS})
#target_link_libraries(sdarray bitarray)
add_sources(mscdsa ${SRCS} ${HEADERS})

add_executable(t_sda sda_test.cpp)
target_link_libraries(t_sda mscdsa)
set_target_properties(t_sda PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(t_rlsum rlsum_test.cpp)
target_link_libraries(t_rlsum mscdsa)
set_target_properties(t_rlsum PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
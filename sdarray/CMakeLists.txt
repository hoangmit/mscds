cmake_minimum_required(VERSION 2.6)

project(sdarray)

set(SRCS sdarray.cpp RLSum.cpp)
set(HEADERS sdarray.h rlsum_trivbin.h RLSum.h)

add_library(sdarray ${SRCS} ${HEADERS})
target_link_libraries(sdarray bitarray)

add_executable(t_sda sda_test.cpp)
target_link_libraries(t_sda bitarray sdarray mem utils)
set_target_properties(t_sda PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(t_rlsum rlsum_test.cpp)
target_link_libraries(t_rlsum sdarray mem utils)
set_target_properties(t_rlsum PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
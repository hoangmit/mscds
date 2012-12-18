cmake_minimum_required(VERSION 2.6)

project(gntf)

set(SRCS RLSum.cpp)
set(HEADERS trivbin.h RLSum.h)

add_executable(t_gntf gntf_test.cpp ${SRCS} ${HEADERS})
target_link_libraries(t_gntf bitarray mem utils)
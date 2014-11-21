cmake_minimum_required(VERSION 2.6)

project(stdio_stream)

set(SRCS ext_comp.cpp
)
set(HEADERS ext_comp.h
)

add_library(stdio_stream ${SRCS} ${HEADERS})
target_link_libraries(stdio_stream)
#add_sources(mscdsa ${SRCS} ${HEADERS})

#add_test_files(x.cpp)
#add_benchmark_files(x.cpp)
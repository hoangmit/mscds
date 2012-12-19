cmake_minimum_required(VERSION 2.6)

project(gntf)

set(SRCS chrfmt.cpp gntf.cpp)
set(HEADERS chrfmt.h gntf.h)
add_sources(mscdsa ${SRCS})

add_executable(bedgraph2gnt bedgraph2gnt.cpp ${HEADERS})
target_link_libraries(bedgraph2gnt mscdsa)
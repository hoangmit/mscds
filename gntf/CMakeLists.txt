cmake_minimum_required(VERSION 2.6)

project(gntf)

set(SRCS chrfmt.cpp gntf.cpp)
set(HEADERS chrfmt.h gntf.h)

add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
target_link_libraries(bedgraph2gnt tree sdarray bitarray mem)